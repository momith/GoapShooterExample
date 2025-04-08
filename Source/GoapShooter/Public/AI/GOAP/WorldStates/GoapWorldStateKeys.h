#pragma once

#include "CoreMinimal.h"
#include "GoapWorldStateKeys.generated.h"

/**
 * Enum for all possible world state keys in the GOAP system.
 * This ensures consistency and prevents typos when referring to world state keys.
 */
UENUM(BlueprintType)
enum class EGoapWorldStateKey : uint8
{
    // Character state
    DummyState UMETA(DisplayName = "Dummy State"),
    //HasWeapon UMETA(DisplayName = "Has Weapon"),
    //HasAmmo UMETA(DisplayName = "Has Ammo"),
    //IsHealthLow UMETA(DisplayName = "Is Health Low"),
    AIPosition UMETA(DisplayName = "AI Position"),
    
    IsCoverNearby UMETA(DisplayName = "Is Cover Nearby"),
    BestCoverLocation UMETA(DisplayName = "Best Cover Location"),
    IsInCover UMETA(DisplayName = "Is In Cover"),

    IsFlankTargetLocationAvailable UMETA(DisplayName = "Is Flank Target Location Available"),

    // Combat state
    CanSeeEnemy UMETA(DisplayName = "Can See Enemy"),
    //IsEnemyDead UMETA(DisplayName = "Is Enemy Dead"),
    //sEnemyInRange UMETA(DisplayName = "Is Enemy In Range"),
    WorldTimeLastEnemySeen UMETA(DisplayName = "World Time Last Enemy Seen"),
    HasLastSeenEnemyLocation UMETA(DisplayName = "Has Last Seen Enemy Location"),
    AllEnemiesInSightAreKilled UMETA(DisplayName = "All Enemies In Sight Are Killed"),
    IsAimedAt UMETA(DisplayName = "Is Aimed At"),
    IsInDanger UMETA(DisplayName = "Is In Danger"),

    // Perception state
    IsFocusedOnStrongPerceivedStimulus UMETA(DisplayName = "Is Focused On Strong Perceived Stimulus"),
    IsMinThresholdFulfilledForEnemyPerception UMETA(DisplayName = "Is Min Threshold Fulfilled For Enemy Perception"),
    IsLookingAtStimulus UMETA(DisplayName = "Is Looking At Stimulus"),
    HasPerceivedStimulus UMETA(DisplayName = "Has Perceived Stimulus"),
    PerceivedStimulusType UMETA(DisplayName = "Perceived Stimulus Type"),
    HasInvestigatedStimulus UMETA(DisplayName = "Has Investigated Stimulus"),
    StimulusInvestigationComplete UMETA(DisplayName = "Stimulus Investigation Complete"),
    PerceivedStimulusLocation UMETA(DisplayName = "Perceived Stimulus Location"),
};

/**
 * Utility class for converting between EGoapWorldStateKey and FString
 */
class GOAPSHOOTER_API FGoapWorldStateKeyUtils
{
public:
    /**
     * Convert a world state key enum to its string representation
     */
    static FString ToString(EGoapWorldStateKey Key)
    {
        const UEnum* EnumPtr = StaticEnum<EGoapWorldStateKey>();
        if (!EnumPtr)
        {
            return FString("Invalid");
        }
        
        // Get the enum name without the enum type prefix
        FString EnumName = EnumPtr->GetNameStringByValue(static_cast<int64>(Key));
        
        // Remove the enum type prefix if present
        static const FString Prefix = TEXT("EGoapWorldStateKey::");
        if (EnumName.StartsWith(Prefix))
        {
            EnumName.RightChopInline(Prefix.Len());
        }
        
        return EnumName;
    }
    
    /**
     * Convert a string to a world state key enum
     */
    static EGoapWorldStateKey FromString(const FString& String)
    {
        const UEnum* EnumPtr = StaticEnum<EGoapWorldStateKey>();
        if (!EnumPtr)
        {
            return EGoapWorldStateKey::DummyState; // Default value
        }
        
        // Try to find the enum value by name
        int64 EnumValue = EnumPtr->GetValueByNameString(String);
        if (EnumValue == INDEX_NONE)
        {
            // Try with the enum type prefix
            EnumValue = EnumPtr->GetValueByNameString(FString::Printf(TEXT("EGoapWorldStateKey::%s"), *String));
        }
        
        if (EnumValue == INDEX_NONE)
        {
            return EGoapWorldStateKey::DummyState; // Default value
        }
        
        return static_cast<EGoapWorldStateKey>(EnumValue);
    }
};
