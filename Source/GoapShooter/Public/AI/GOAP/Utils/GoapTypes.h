#pragma once

#include "CoreMinimal.h"
#include "GoapTypes.generated.h"

/**
 * Enum for different types of GOAP values
 */
UENUM(BlueprintType)
enum class EGoapValueType : uint8
{
    Boolean UMETA(DisplayName = "Boolean"),
    Integer UMETA(DisplayName = "Integer"),
    Float UMETA(DisplayName = "Float"),
    String UMETA(DisplayName = "String"),
    Object UMETA(DisplayName = "Object"),
    Vector UMETA(DisplayName = "Vector")
};

/**
 * Wrapper class to store different types of values for GOAP world state
 */
USTRUCT(BlueprintType)
struct FGoapValue
{
	GENERATED_BODY()

	FGoapValue() : BoolValue(false), IntValue(0), FloatValue(0.0f), StringValue(""), ObjectValue(nullptr), VectorValue(FVector::ZeroVector), ValueType(EGoapValueType::Boolean) {}

	explicit FGoapValue(bool Value) : BoolValue(Value), IntValue(0), FloatValue(0.0f), StringValue(""), ObjectValue(nullptr), VectorValue(FVector::ZeroVector), ValueType(EGoapValueType::Boolean) {}
	explicit FGoapValue(int32 Value) : BoolValue(false), IntValue(Value), FloatValue(0.0f), StringValue(""), ObjectValue(nullptr), VectorValue(FVector::ZeroVector), ValueType(EGoapValueType::Integer) {}
	explicit FGoapValue(float Value) : BoolValue(false), IntValue(0), FloatValue(Value), StringValue(""), ObjectValue(nullptr), VectorValue(FVector::ZeroVector), ValueType(EGoapValueType::Float) {}
	explicit FGoapValue(const FString& Value) : BoolValue(false), IntValue(0), FloatValue(0.0f), StringValue(Value), ObjectValue(nullptr), VectorValue(FVector::ZeroVector), ValueType(EGoapValueType::String) {}
	explicit FGoapValue(UObject* Value) : BoolValue(false), IntValue(0), FloatValue(0.0f), StringValue(""), ObjectValue(Value), VectorValue(FVector::ZeroVector), ValueType(EGoapValueType::Object) {}
	explicit FGoapValue(const FVector& Value) : BoolValue(false), IntValue(0), FloatValue(0.0f), StringValue(""), ObjectValue(nullptr), VectorValue(Value), ValueType(EGoapValueType::Vector) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	bool BoolValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	int32 IntValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float FloatValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	UObject* ObjectValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	FVector VectorValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	EGoapValueType ValueType;

	bool operator==(const FGoapValue& Other) const
	{
		if (ValueType != Other.ValueType)
			return false;

		switch (ValueType)
		{
		case EGoapValueType::Boolean:
			return BoolValue == Other.BoolValue;
		case EGoapValueType::Integer:
			return IntValue == Other.IntValue;
		case EGoapValueType::Float:
			return FMath::IsNearlyEqual(FloatValue, Other.FloatValue);
		case EGoapValueType::String:
			return StringValue.Equals(Other.StringValue);
		case EGoapValueType::Object:
			return ObjectValue == Other.ObjectValue;
		case EGoapValueType::Vector:
			return VectorValue.Equals(Other.VectorValue, 0.1f);
		default:
			return false;
		}
	}

	bool operator!=(const FGoapValue& Other) const
	{
		return !(*this == Other);
	}
};
