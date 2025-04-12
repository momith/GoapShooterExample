#include "AI/EQS/EnvQuery_FindCover.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryOption.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_PathingGrid.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Overlap.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Pathfinding.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Trace.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Distance.h"
#include "AI/EQS/EnvQueryContext_AllOtherPlayers.h"

UEnvQuery_FindCover::UEnvQuery_FindCover(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create a "query option" (combines a query generator with query tests)
	UEnvQueryOption* QueryOption = NewObject<UEnvQueryOption>();

	// Create a pathing grid generator (will create points on the navigable area by creating a grid)
	UEnvQueryGenerator_PathingGrid* GridGenerator = NewObject<UEnvQueryGenerator_PathingGrid>(QueryOption);
	//GridGenerator->ItemType is UEnvQueryItemType_Point::StaticClass() because were searching for points/locations

	// Set grid parameters (size, spacing, etc.)
	GridGenerator->GridSize.DefaultValue = 1000.0f;
	GridGenerator->SpaceBetween.DefaultValue = 100.0f;
	GridGenerator->GenerateAround = UEnvQueryContext_Querier::StaticClass();
	GridGenerator->ProjectionData.PostProjectionVerticalOffset = 40.0f; // moves everything a little bit up, so the sphere center is not at 0
	
	QueryOption->Generator = GridGenerator;

	// Add test: Check if when spawning a sphere at the point there would be overlap with a static mesh (e.g. wall)
	// This way we will filter points which are adjacent to a wall
	UEnvQueryTest_Overlap* OverlapTest = NewObject<UEnvQueryTest_Overlap>(QueryOption);
	OverlapTest->TestPurpose = EEnvTestPurpose::Filter;
	OverlapTest->OverlapData.ExtentX = 100.0f;
	OverlapTest->OverlapData.ExtentY = 10.0f;
	OverlapTest->OverlapData.ExtentZ = 10.0f;
	OverlapTest->OverlapData.ShapeOffset = FVector(0.0f, 0.0f, 120.0f);
	OverlapTest->OverlapData.OverlapShape = EEnvOverlapShape::Sphere;
	OverlapTest->OverlapData.bOnlyBlockingHits = true;
	OverlapTest->OverlapData.bOverlapComplex = false;
	OverlapTest->OverlapData.bSkipOverlapQuerier = true;

	QueryOption->Tests.Add(OverlapTest);
	
	// Add test: Check that a line trace towards other players will get blocked by some static mesh (e.g. wall)
	UEnvQueryTest_Trace* TraceTest = NewObject<UEnvQueryTest_Trace>(QueryOption);
	TraceTest->TestPurpose = EEnvTestPurpose::Filter;
	TraceTest->TraceData.TraceMode = EEnvQueryTrace::GeometryByChannel; // Search for geometry
	TraceTest->TraceData.TraceChannel = ETraceTypeQuery::TraceTypeQuery1; // Visibility channel
	TraceTest->TraceData.TraceShape = EEnvTraceShape::Sphere; // Use a sphere to make it more precise
	TraceTest->TraceData.ExtentX = 30.0f; // Sphere size
	TraceTest->TraceData.bTraceComplex = false;
	TraceTest->TraceData.bOnlyBlockingHits = true;
	TraceTest->Context = UEnvQueryContext_AllOtherPlayers::StaticClass();
	TraceTest->ItemHeightOffset.DefaultValue = 90.0f; // approximately eye level at point/location to test
	TraceTest->ContextHeightOffset.DefaultValue = 90.0f; // approximately eye level at player to trace towards to

	QueryOption->Tests.Add(TraceTest);
	
	// Add test: Minimum distance to other players
	// TODO This might be not necessary
	UEnvQueryTest_Distance* MinDistanceToEnemyTest = CreateDistanceTest(QueryOption);
	MinDistanceToEnemyTest->TestPurpose = EEnvTestPurpose::Filter;
	MinDistanceToEnemyTest->TestMode = EEnvTestDistance::Distance3D;
	MinDistanceToEnemyTest->DistanceTo = UEnvQueryContext_AllOtherPlayers::StaticClass();
	MinDistanceToEnemyTest->FilterType = EEnvTestFilterType::Minimum;
	MinDistanceToEnemyTest->FloatValueMin.DefaultValue = 300.0f; // if too near to other player, its not considered as worth it
	
	QueryOption->Tests.Add(MinDistanceToEnemyTest);
	
	// // Add test: Points that are more near to other player shall be scored better
	// // Does this make sense?!
	// UEnvQueryTest_Distance* PreferLessDistanceTest = NewObject<UEnvQueryTest_Distance>(QueryOption);
	// PreferLessDistanceTest->TestPurpose = EEnvTestPurpose::Score;
	// PreferLessDistanceTest->TestMode = EEnvTestDistance::Distance3D;
	// PreferLessDistanceTest->DistanceTo = UEnvQueryContext_AllOtherPlayers::StaticClass();
	// PreferLessDistanceTest->ScoringEquation = EEnvTestScoreEquation::InverseLinear;
	// PreferLessDistanceTest->ScoringFactor.DefaultValue = 1.0f;

	// Add test: Points that are more near to querier shall be scored better (less effort)
	UEnvQueryTest_Distance* PreferLessDistanceTest = CreateDistanceTest(QueryOption);
	PreferLessDistanceTest->TestPurpose = EEnvTestPurpose::Score;
	PreferLessDistanceTest->TestMode = EEnvTestDistance::Distance3D;
	PreferLessDistanceTest->DistanceTo = UEnvQueryContext_Querier::StaticClass();
	PreferLessDistanceTest->ScoringEquation = EEnvTestScoreEquation::InverseLinear; // the lower the distance, the higher the score
	
	QueryOption->Tests.Add(PreferLessDistanceTest);
	
	// Add test: Path shall exist
	UEnvQueryTest_Pathfinding* PathExistsTest = NewObject<UEnvQueryTest_Pathfinding>(QueryOption);
	PathExistsTest->TestPurpose = EEnvTestPurpose::Filter;
	PathExistsTest->TestMode = EEnvTestPathfinding::PathExist;
	PathExistsTest->Context = UEnvQueryContext_Querier::StaticClass();

	QueryOption->Tests.Add(PathExistsTest);
	
	// Add test: Minimum distance from querier
	// TODO This might be not necessary or could even be rather problematic
	UEnvQueryTest_Distance* MinDistanceTest = CreateDistanceTest(QueryOption);
	MinDistanceTest->TestPurpose = EEnvTestPurpose::Filter;
	MinDistanceTest->TestMode = EEnvTestDistance::Distance3D;
	MinDistanceTest->DistanceTo = UEnvQueryContext_Querier::StaticClass();
	MinDistanceTest->FilterType = EEnvTestFilterType::Minimum;
	MinDistanceTest->FloatValueMin.DefaultValue = 80.0f; // if too near, its not considered as worth it
	
	QueryOption->Tests.Add(MinDistanceTest);

	// Add the configured generator/test combination to the query
	Options.Add(QueryOption);
}

UEnvQueryTest_Distance* UEnvQuery_FindCover::CreateDistanceTest(UObject* Outer)
{
	// for some weird reason cannot be constructed like the others (linker error)
	// see also https://forums.unrealengine.com/t/runtime-editable-environment-queries-from-c-wont-build/364818/3
	UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("/Script/AIModule.EnvQueryTest_Distance"));
	UEnvQueryTest* DistanceTest = NewObject<UEnvQueryTest>(Outer, Class);
	UEnvQueryTest_Distance* DistanceTestWrapper = (UEnvQueryTest_Distance*) DistanceTest;
	return DistanceTestWrapper;
}
