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
    
    /** Strength of the stimulus (0.0 to 1.0, or 1.5 for damage) */
    UPROPERTY(BlueprintReadWrite, Category = "GOAP|Perception")
    float Strength = 0.0f; // to be refactored: maybe rename to InitialStrength? or make it private? should be differentiated from GetStrength()
    
    /** Whether this perception has been investigated */
    UPROPERTY(BlueprintReadWrite, Category = "GOAP|Perception")
    bool bHasBeenInvestigated = false;

    UPROPERTY()
    float SecondsUntilRemovalOfPerceptionData = 20.0f;

    FPerceptionData() {}

    float GetStrength() const
    {
        if (!PerceivedActor) // invalid actor => no strength
        {
            return 0.0f;
        }
        if (LastPerceivedTime == FLT_MAX) // actively perceived => stimuli strength
        {
            return Strength;
        }
        if (PerceptionType == EPerceptionType::Audio)
        {
            // audio expired -> strength decreases linear over time relatively fast
            float WorldTime = PerceivedActor->GetWorld()->GetTimeSeconds();
            return Strength * (1.0f - (WorldTime - LastPerceivedTime) / (SecondsUntilRemovalOfPerceptionData * 0.33f));
        }
        // expired => strength decreases linear over time
        float WorldTime = PerceivedActor->GetWorld()->GetTimeSeconds();
        return Strength * (1.0f - (WorldTime - LastPerceivedTime) / SecondsUntilRemovalOfPerceptionData);
    }
};
