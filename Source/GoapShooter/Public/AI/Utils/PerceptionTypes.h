#pragma once

#include "CoreMinimal.h"
#include "PerceptionTypes.generated.h"

/**
 * Enum for different types of perception stimuli in the GOAP system.
 */
UENUM(BlueprintType)
enum class EPerceptionType : uint8
{
    None UMETA(DisplayName = "None"),
    Visual UMETA(DisplayName = "Visual"),
    Audio UMETA(DisplayName = "Audio"),
    Damage UMETA(DisplayName = "Damage"),
};

/**
 * Utility class for converting between EPerceptionType and FString
 */
class GOAPSHOOTER_API FPerceptionTypeUtils
{
public:
    /**
     * Convert a perception type enum to its string representation
     */
    static FString ToString(EPerceptionType Type)
    {
        const UEnum* EnumPtr = StaticEnum<EPerceptionType>();
        if (!EnumPtr)
        {
            return FString("Invalid");
        }
        
        static const FString Prefix = TEXT("EPerceptionType::");
        FString EnumString = EnumPtr->GetNameByValue(static_cast<int64>(Type)).ToString();
        
        if (EnumString.StartsWith(Prefix))
        {
            EnumString.RemoveFromStart(Prefix);
        }
        
        return EnumString;
    }
    
    /**
     * Convert a string to its corresponding perception type enum
     */
    static EPerceptionType FromString(const FString& String)
    {
        const UEnum* EnumPtr = StaticEnum<EPerceptionType>();
        if (!EnumPtr)
        {
            return EPerceptionType::None; // Default value
        }
        
        // Try to find the enum value
        int64 EnumValue;
        if (String.IsEmpty())
        {
            return EPerceptionType::None;
        }
        else
        {
            EnumValue = EnumPtr->GetValueByNameString(FString::Printf(TEXT("EPerceptionType::%s"), *String));
        }
        
        if (EnumValue == INDEX_NONE)
        {
            return EPerceptionType::None; // Default value
        }
        
        return static_cast<EPerceptionType>(EnumValue);
    }
};

/**
 * Structure to hold perception data
 */
USTRUCT(BlueprintType)
struct GOAPSHOOTER_API FPerceptionData
{
    GENERATED_BODY()
    
    /** The actor that was perceived */
    UPROPERTY(BlueprintReadWrite, Category = "GOAP|Perception")
    AActor* PerceivedActor = nullptr;
    
    /** Location where the stimulus was detected */
    UPROPERTY(BlueprintReadWrite, Category = "GOAP|Perception")
    FVector LastPerceivedLocation = FVector::ZeroVector;
    
    /** World time when the stimulus was detected */
    UPROPERTY(BlueprintReadWrite, Category = "GOAP|Perception")
    float LastPerceivedTime = -FLT_MAX;

    /** Type of perception */
    UPROPERTY(BlueprintReadWrite, Category = "GOAP|Perception")
    EPerceptionType PerceptionType = EPerceptionType::None;
    
    /** Strength of the stimulus (0.0 to 1.0) */
    UPROPERTY(BlueprintReadWrite, Category = "GOAP|Perception")
    float Strength = 0.0f;
    
    /** Whether this perception has been investigated */
    UPROPERTY(BlueprintReadWrite, Category = "GOAP|Perception")
    bool bHasBeenInvestigated = false;
    
    FPerceptionData() {}
};
