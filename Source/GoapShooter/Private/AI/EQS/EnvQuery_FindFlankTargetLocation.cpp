#include "AI/EQS/EnvQuery_FindFlankTargetLocation.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryOption.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_PathingGrid.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Overlap.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Trace.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Distance.h"
#include "EQS/EnvQueryContext_MostPerceivedEnemy.h"
#include "EQS/EnvQueryTest_IsBehindActor.h"
#include "EQS/EnvQueryTest_PathNearActor.h"

UEnvQuery_FindFlankTargetLocation::UEnvQuery_FindFlankTargetLocation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
		// Create a "query option" (combines a query generator with query tests)
	UEnvQueryOption* QueryOption = NewObject<UEnvQueryOption>();

	// Create a pathing grid generator (will create points on the navigable area by creating a grid)
	UEnvQueryGenerator_PathingGrid* GridGenerator = NewObject<UEnvQueryGenerator_PathingGrid>(QueryOption);
	//GridGenerator->ItemType is UEnvQueryItemType_Point::StaticClass() because were searching for points/locations

	// Set grid parameters (size, spacing, etc.)
	GridGenerator->GridSize.DefaultValue = 1500.0f;
	GridGenerator->SpaceBetween.DefaultValue = 100.0f;
	GridGenerator->GenerateAround = UEnvQueryContext_MostPerceivedEnemy::StaticClass();
	GridGenerator->ProjectionData.PostProjectionVerticalOffset = 40.0f; // might be not necessary
	
	QueryOption->Generator = GridGenerator;

	// // Add test: Same side as querier relative to most perceived enemy?
	// // Made more trouble than it was worth it
	// UEnvQueryTest_SameSideAsQuerier* SameSideAsQuerierTest = NewObject<UEnvQueryTest_SameSideAsQuerier>(QueryOption);
	// SameSideAsQuerierTest->TestPurpose = EEnvTestPurpose::Filter;
	// SameSideAsQuerierTest->ActorContext = UEnvQueryContext_MostPerceivedEnemy::StaticClass();
	// SameSideAsQuerierTest->bWantSameSide = true;
	// SameSideAsQuerierTest->bUse2DCalculation = true;
	// SameSideAsQuerierTest->bUseRightVector = true;
	//
	// QueryOption->Tests.Add(SameSideAsQuerierTest);
	
	// Add test: Check that the location is outside the field of view of the enemy and not directly behind him (=> somewhere left or right)
	UEnvQueryTest_IsBehindActor* IsBehindEnemyTest = NewObject<UEnvQueryTest_IsBehindActor>(QueryOption);
	IsBehindEnemyTest->TestPurpose = EEnvTestPurpose::Filter;
	IsBehindEnemyTest->ActorContext = UEnvQueryContext_MostPerceivedEnemy::StaticClass();
	IsBehindEnemyTest->BehindAngleThreshold = 90.0f;
	IsBehindEnemyTest->MinimumDistanceBehind = 100.0f;
	IsBehindEnemyTest->bUse2DDistance = true;
	IsBehindEnemyTest->bExpected = true;
	IsBehindEnemyTest->bInvertDirection = false;

	QueryOption->Tests.Add(IsBehindEnemyTest);

	UEnvQueryTest_IsBehindActor* IsBehindEnemyTest2 = NewObject<UEnvQueryTest_IsBehindActor>(QueryOption);
	IsBehindEnemyTest2->TestPurpose = EEnvTestPurpose::Filter;
	IsBehindEnemyTest2->ActorContext = UEnvQueryContext_MostPerceivedEnemy::StaticClass();
	IsBehindEnemyTest2->BehindAngleThreshold = 140.0f;
	IsBehindEnemyTest2->MinimumDistanceBehind = 100.0f;
	IsBehindEnemyTest2->bUse2DDistance = true;
	IsBehindEnemyTest2->bExpected = true;
	IsBehindEnemyTest2->bInvertDirection = true;

	QueryOption->Tests.Add(IsBehindEnemyTest2);
	
	// Add test: Check that a line trace towards other players will NOT get blocked by some static mesh (e.g. wall)
	UEnvQueryTest_Trace* TraceTest = NewObject<UEnvQueryTest_Trace>(QueryOption);
	TraceTest->TestPurpose = EEnvTestPurpose::Filter;
	TraceTest->TraceData.TraceMode = EEnvQueryTrace::GeometryByChannel; // Search for geometry
	TraceTest->TraceData.TraceChannel = ETraceTypeQuery::TraceTypeQuery1; // Visibility channel
	TraceTest->TraceData.TraceShape = EEnvTraceShape::Line;
	TraceTest->TraceData.bTraceComplex = false;
	TraceTest->TraceData.bOnlyBlockingHits = true;
	TraceTest->Context = UEnvQueryContext_MostPerceivedEnemy::StaticClass();
	TraceTest->ItemHeightOffset.DefaultValue = 80.0f; // approximately eye level at point/location to test
	TraceTest->ContextHeightOffset.DefaultValue = 80.0f; // approximately eye level at player to trace towards to
	TraceTest->BoolValue.DefaultValue = false; // NEGATE IT! we want to filter these where a trace hits!
	
	QueryOption->Tests.Add(TraceTest);

	// Add test: We dont want a location where we would need to pass by near the enemy.
	//   Actually this test is rather heuristic, because the used path finding algorithm finds the shortest path, but
	//   in the FlankingComponent we will spawn/place a nav blocker which changes the nav area environment.
	//   unfortunately its not easily possible to put this logic inside the EQS test
	UEnvQueryTest_PathNearActor* PathNearEnemyTest = NewObject<UEnvQueryTest_PathNearActor>(QueryOption);
	PathNearEnemyTest->TestPurpose = EEnvTestPurpose::Filter;
	PathNearEnemyTest->ActorContext = UEnvQueryContext_MostPerceivedEnemy::StaticClass();
	PathNearEnemyTest->ProximityThreshold = 300.0f;
	PathNearEnemyTest->bWantPathsNearActor = false;
	PathNearEnemyTest->bUse2DDistance = true;
	PathNearEnemyTest->MaxPathPointsToCheck = 10.0f;
	PathNearEnemyTest->bUseSimpleDistanceCheck = false;

	QueryOption->Tests.Add(PathNearEnemyTest);
	
	// Add test: We want to prefer locations which are farther away from the enemy
	// TODO maybe a custom score equation would be better
	UEnvQueryTest_Distance* PreferMoreDistanceTest = CreateDistanceTest(QueryOption);
	PreferMoreDistanceTest->TestPurpose = EEnvTestPurpose::Score;
	PreferMoreDistanceTest->TestMode = EEnvTestDistance::Distance3D;
	PreferMoreDistanceTest->DistanceTo = UEnvQueryContext_MostPerceivedEnemy::StaticClass();
	PreferMoreDistanceTest->ClampMinType = EEnvQueryTestClamping::SpecifiedValue;
	PreferMoreDistanceTest->ScoreClampMin.DefaultValue = 100.0f;
	PreferMoreDistanceTest->ClampMaxType = EEnvQueryTestClamping::SpecifiedValue;
	PreferMoreDistanceTest->ScoreClampMax.DefaultValue = 600.0f;
	PreferMoreDistanceTest->ScoringEquation = EEnvTestScoreEquation::SquareRoot;
	
	QueryOption->Tests.Add(PreferMoreDistanceTest);

	// Add the configured generator/test combination to the query
	Options.Add(QueryOption);
}

UEnvQueryTest_Distance* UEnvQuery_FindFlankTargetLocation::CreateDistanceTest(UObject* Outer)
{
	// for some weird reason cannot be constructed like the others (linker error)
	// see also https://forums.unrealengine.com/t/runtime-editable-environment-queries-from-c-wont-build/364818/3
	UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("/Script/AIModule.EnvQueryTest_Distance"));
	UEnvQueryTest* DistanceTest = NewObject<UEnvQueryTest>(Outer, Class);
	UEnvQueryTest_Distance* DistanceTestWrapper = (UEnvQueryTest_Distance*) DistanceTest;
	return DistanceTestWrapper;
}
