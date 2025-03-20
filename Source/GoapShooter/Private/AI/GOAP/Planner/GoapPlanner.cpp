#include "AI/GOAP/Planner/GoapPlanner.h"
#include "Algo/Reverse.h"

UGoapPlanner::UGoapPlanner()
{
}

void UGoapPlanner::AddAction(UGoapAction* Action)
{
    if (Action)
    {
        Actions.Add(Action);
    }
}

void UGoapPlanner::AddGoal(UGoapGoal* Goal)
{
    if (Goal)
    {
        Goals.Add(Goal);
    }
}

void UGoapPlanner::UpdateWorldState(const FString& Key, const FGoapValue& Value)
{
    WorldState.Add(Key, Value);
}

void UGoapPlanner::FullUpdateWorldState(TMap<FString, FGoapValue> WorldStateToSet)
{
    WorldState = WorldStateToSet;
}

UGoapGoal* UGoapPlanner::SelectBestGoal()
{
    // Find the goal with the highest priority
    UGoapGoal* BestGoal = nullptr;
    int32 HighestPriority = TNumericLimits<int32>::Lowest();

    for (UGoapGoal* Goal : Goals)
    {
        if (!Goal->ShallGoalBeConsidered(WorldState))
        {
            continue;
        }
        int32 GoalPriority = Goal->GetPriority(WorldState);
        if (GoalPriority > HighestPriority)
        {
            BestGoal = Goal;
            HighestPriority = GoalPriority;
        }
    }

    return BestGoal;
}

UGoapAction* UGoapPlanner::PlanNextAction(UGoapGoal* Goal)
{
    // remove disabled actions temporarily from list
    TArray<UGoapAction*> DisabledActions;
    for (UGoapAction* Action : Actions)
    {
        if (!Action->ShallActionBeConsidered(WorldState))
        {
            DisabledActions.Add(Action);
        }
    }
    for (UGoapAction* Action : DisabledActions)
    {
        Actions.Remove(Action);
    }

    // Do A* planning to find the best sequence of actions (returning only the first of the sequence)
    UGoapAction* NextAction = AStarPlanning(WorldState, Goal);
    
    // add disabled actions back to list
    Actions.Append(DisabledActions);

    // return the first action of the sequence
    return NextAction;
}

UGoapAction* UGoapPlanner::AStarPlanning(const TMap<FString, FGoapValue>& StartState, UGoapGoal* Goal)
{
    // Use TArray as a priority queue
    TArray<TSharedPtr<FGoapNode>> OpenSet;
    
    // Use TSet for visited states
    TSet<uint32> VisitedStateHashes;
    
    // Create start node
    TSharedPtr<FGoapNode> StartNode = MakeShared<FGoapNode>(StartState, nullptr, nullptr, 0.0f, Heuristic(StartState, Goal));
    OpenSet.Add(StartNode);
    
    while (OpenSet.Num() > 0)
    {
        // Sort the array to find the node with lowest F cost
        OpenSet.Sort(FGoapNodeComparator());
        
        // Get the node with lowest F cost
        TSharedPtr<FGoapNode> CurrentNode = OpenSet[0];
        OpenSet.RemoveAt(0);
        
        // Check if goal is satisfied
        if (Goal->IsSatisfiedBy(CurrentNode->State))
        {
            return ExtractFirstAction(CurrentNode);
        }
        
        // Mark as visited
        VisitedStateHashes.Add(CurrentNode->GetStateHash());
        
        // Try all actions
        for (UGoapAction* Action : Actions)
        {
            if (!Action || !Action->ArePreconditionsSatisfied(CurrentNode->State))
            {
                continue;
            }
            
            // Apply action effects to get new state
            TMap<FString, FGoapValue> NewState = Action->ApplyEffects(CurrentNode->State);
            
            // Create new node
            TSharedPtr<FGoapNode> NewNode = MakeShared<FGoapNode>(
                NewState, 
                CurrentNode, 
                Action, 
                CurrentNode->GCost + Action->GetCost(CurrentNode->State), 
                Heuristic(NewState, Goal)
            );
            
            // Check if state was already visited
            uint32 NewStateHash = NewNode->GetStateHash();
            if (VisitedStateHashes.Contains(NewStateHash))
            {
                continue;
            }
            
            // Add to open set
            OpenSet.Add(NewNode);
        }
    }
    
    // No plan found
    return nullptr;
}

UGoapAction* UGoapPlanner::ExtractFirstAction(const TSharedPtr<FGoapNode>& Node)
{
    if (!Node)
    {
        return nullptr;
    }
    
    // Trace back to the first action
    TSharedPtr<FGoapNode> CurrentNode = Node;
    TSharedPtr<FGoapNode> ParentNode = CurrentNode->Parent;
    
    // If we're already at the start node, return the action
    if (!ParentNode)
    {
        return CurrentNode->Action;
    }
    
    // Go back until we find the first action
    while (ParentNode && ParentNode->Parent)
    {
        CurrentNode = ParentNode;
        ParentNode = ParentNode->Parent;
    }
    
    return CurrentNode->Action;
}

float UGoapPlanner::Heuristic(const TMap<FString, FGoapValue>& State, UGoapGoal* Goal)
{
    if (!Goal)
    {
        return 0.0f;
    }
    
    // Count the number of unsatisfied conditions
    float UnsatisfiedCount = 0.0f;
    
    for (const auto& Pair : Goal->GetDesiredState())
    {
        const FString& Key = Pair.Key;
        const FGoapValue& DesiredValue = Pair.Value;
        
        if (!State.Contains(Key) || State[Key] != DesiredValue)
        {
            UnsatisfiedCount += 1.0f;
        }
    }
    
    return UnsatisfiedCount;
}
