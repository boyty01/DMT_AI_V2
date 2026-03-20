/**
 * DMT_AI_V2 Plugin Documentation Data
 * Loaded by index.html before renderer.js.
 *
 * To document a new plugin, copy this file, replace all content inside
 * window.PLUGIN_DATA = { ... }, and update the <script> src in index.html.
 *
 * ── SCHEMA REFERENCE ──────────────────────────────────────────────────────
 *
 * plugin        { name, module, description, logoText?, namePrefix?, nameSuffix? }
 * overview      { title, subtitle, body?, callouts?: [{type:"note|tip|warn", text}] }
 * architecture  { body?, layers: [{num, name, items:[]}] }
 * workflows     [{ title, steps:[] }]
 * groups        [{ id, label, classes:["ClassName",...] }]
 *   — defines sidebar sections and the order classes appear in each section
 * classes       [{ id, name, parents:[], description, navDot, badges:[{text,cls}],
 *                  body:[memberGroup], callouts?:[{type,text}] }]
 *   memberGroup types:
 *     propertyTable  { title, type:"propertyTable",  rows:[{name,type,default?,desc,meta?}] }
 *     functionTable  { title, type:"functionTable",  rows:[{sig,desc,meta?}] }
 *     enumBlock      { title, type:"enumBlock",      enums:[{name,type?,values:[{name,desc}]}] }
 *     structBlock    { title, type:"structBlock",    structs:[{name,fields:[{name,desc}]}] }
 *     delegateList   { title, type:"delegateList",   delegates:[{sig,desc}] }
 *     calloutGroup   { title, type:"calloutGroup",   callouts:[{type,text}] }
 *     paragraphs     { title, type:"paragraphs",     paragraphs:[] }
 *   meta tags (on rows): [{text:"BlueprintCallable", cls:"meta-bp"}, ...]
 *     cls options: meta-bp, meta-pure, meta-native, meta-impl, meta-prot, meta-cat
 * delegatesRef  [{ sig, desc }]
 *
 * In desc/text strings: `code` renders as <code>code</code>
 *                        **text** renders as <strong>text</strong>
 */

window.PLUGIN_DATA = {

    // ── PLUGIN METADATA ───────────────────────────────────────────────────────
    plugin: {
        name:       "DMT_AI_V2",
        module:     "DMT_AI_V2",
        description:"Unreal Engine 5 — C++ Plugin",
        logoText:   "AI",
        namePrefix: "DMT",
        nameSuffix: "_AI_V2"
    },

    // ── OVERVIEW ──────────────────────────────────────────────────────────────
    overview: {
        title:    "DMT_AI_V2 Plugin",
        subtitle: "Unreal Engine 5 — C++ Plugin Reference | Module: `DMT_AI_V2`",
        body:     "DMT_AI_V2 is a modular, perception-driven NPC AI plugin for Unreal Engine 5. It provides a layered system covering faction-aware perception, state machine behaviour, Dijkstra pathfinding across node graphs, sequential route traversal, async node actions, and NPC collision avoidance — all fully exposed to Blueprint.",
        callouts: [
            {
                type: "tip",
                text: "All primary classes are **BlueprintSpawnable** or **Blueprintable**. The intended workflow is to subclass scripts and node actions in Blueprint while keeping movement and perception logic in C++."
            },
            {
                type: "note",
                text: "Actors that participate in the perception system must implement `IDmtAiInterfaceV2` to expose faction and character data. Without it, the script cannot classify perceived actors."
            }
        ]
    },

    // ── ARCHITECTURE ──────────────────────────────────────────────────────────
    architecture: {
        body: "The plugin is organised into eight conceptual layers. Higher layers depend on lower ones; lower layers have no upward dependencies.",
        layers: [
            { num: 1, name: "Perception & Orchestration", items: ["UDmtAiBehaviorComponent", "UAIPerceptionComponent"] },
            { num: 2, name: "State Machine",               items: ["UDmtAiV2ScriptBase", "UDmtAiScriptV2Common", "UDmtAiScriptV2Simple"] },
            { num: 3, name: "Perception Data",             items: ["FAiTrackedActorData", "FCharacterAiData", "TMap faction buckets"] },
            { num: 4, name: "Pathfinding",                 items: ["AAiPathNode", "AAiPatrolPath", "UNodeGraphHelper"] },
            { num: 5, name: "Route Movement",              items: ["UAiPathFollowComponent", "UAIController"] },
            { num: 6, name: "Node Actions",                items: ["UNodeAction", "BP_ExecuteAction overrides"] },
            { num: 7, name: "Interfaces",                  items: ["IDmtAiInterfaceV2", "IDmtAiPatrolInterface"] },
            { num: 8, name: "Utilities",                   items: ["UAiFunctionLibrary", "UAiPathScriptBase"] }
        ]
    },

    // ── WORKFLOWS ─────────────────────────────────────────────────────────────
    workflows: [
        {
            title: "NPC Initialisation",
            steps: [
                "Pawn is possessed by an `AAIController` with `UAIPerceptionComponent` configured.",
                "`UDmtAiBehaviorComponent` is attached to the pawn.",
                "On `BeginPlay`, the component caches the perception component and instantiates `ScriptClass`.",
                "Script's `Init()` is called, binding to the behavior component's perception delegate.",
                "`TickComponent` broadcasts `ScriptTick` to the script each frame."
            ]
        },
        {
            title: "Perception Update & Faction Routing",
            steps: [
                "`UAIPerceptionComponent` detects a stimulus (sight, hearing, or damage).",
                "Behavior component receives `OnPerceptionTargetUpdate` and forwards to the script.",
                "Script queries the perceived actor for `IDmtAiInterfaceV2::GetFaction()`.",
                "Faction tag is matched against `HostileFactions`, `FriendlyFactions`, or treated as neutral.",
                "The appropriate `OnHostile/Neutral/FriendlyTargetPerceived` BlueprintNativeEvent fires.",
                "Actor is inserted into the matching `TMap` faction bucket as `FAiTrackedActorData`."
            ]
        },
        {
            title: "AI State Transition",
            steps: [
                "Script calls `AssessAIState()` (gated by `StateAssessmentCooldown` in Common variant).",
                "`HasAnyAttackTargets()` checks for hostiles with valid line-of-sight (Common) or any hostile (Simple).",
                "`HasAnyAlertTargets()` checks for any tracked hostile regardless of LOS.",
                "`SetCurrentAIState()` transitions `ENPCAIState` and fires the matching delegate.",
                "Blueprint behavior trees or event graphs respond to `OnTransitionToAttack/Alert/Patrol`."
            ]
        },
        {
            title: "Patrol Route Execution",
            steps: [
                "Call `UAiPathFollowComponent::FollowNodeRoute(Route)` with an ordered node array.",
                "Component caches `OwnerPawn` and `OwnerController`, resets all counters.",
                "`MoveToNextNode()` binds to `PFC::OnRequestFinished` and issues `MoveToActor`.",
                "A stuck-detection timer samples position every `StuckCheckInterval` seconds.",
                "If movement stalls below `StuckDistanceThreshold`, `YieldBriefly()` pauses the NPC for a randomised duration to let other agents pass.",
                "On successful arrival, `AAiPathNode::OnArrived` executes attached `UNodeAction` components in sequence.",
                "After all actions complete, movement to the next node is deferred one tick before issuing `MoveToActor`, preventing re-entrancy into the PathFollowingComponent.",
                "Repeat until the route is exhausted, then `bIsFollowing` is cleared."
            ]
        },
        {
            title: "Pause & Resume Path Follow",
            steps: [
                "Call `PausePathFollow()` at any time during active route traversal.",
                "All timers (retry, stuck-check, yield) are cleared and `StopMovement()` is called.",
                "`NodeRoute` and `CurrentNodeIndex` are preserved — the NPC remembers where it was heading.",
                "Call `ResumePathFollow()` when ready; the component resumes movement toward the same node that was the active target at pause time, with no backtracking."
            ]
        },
        {
            title: "Custom Node Action (Blueprint)",
            steps: [
                "Create a Blueprint subclass of `UNodeAction`.",
                "Override `BP_ExecuteAction(Pawn)` and implement the desired behaviour (play animation, trigger dialogue, wait, etc.).",
                "Call `OnFinished()` when the action is complete to advance the action chain.",
                "Optionally override `CanExecute(Pawn)` to conditionally skip the action.",
                "Add the component to any `AAiPathNode` actor in the level; it will execute automatically on arrival."
            ]
        }
    ],

    // ── SIDEBAR NAV GROUPS ────────────────────────────────────────────────────
    // Each entry defines a collapsible sidebar section and its class order.
    groups: [
        { id: "actors-heading",     label: "Actors",      classes: ["AAiPathNode", "AAiPatrolPath"] },
        { id: "components-heading", label: "Components",  classes: ["UAiPathFollowComponent", "UDmtAiBehaviorComponent"] },
        { id: "scripts-heading",    label: "Scripts",     classes: ["UDmtAiV2ScriptBase", "UDmtAiScriptV2Common", "UDmtAiScriptV2Simple", "UAiPathScriptBase"] },
        { id: "objects-heading",    label: "Objects",     classes: ["UNodeAction", "UNodeGraphHelper"] },
        { id: "interfaces-heading", label: "Interfaces",  classes: ["IDmtAiInterfaceV2", "IDmtAiPatrolInterface"] },
        { id: "library-heading",    label: "Library",     classes: ["UAiFunctionLibrary"] }
    ],

    // ── CLASSES ───────────────────────────────────────────────────────────────
    classes: [

        // ── AAiPathNode ────────────────────────────────────────────────────────
        {
            id:          "AAiPathNode",
            name:        "AAiPathNode",
            parents:     ["AActor"],
            navDot:      "dot-actor",
            description: "Waypoint actor placed in the level to define AI navigation graphs. Nodes store an acceptance radius, a list of connected neighbours for Dijkstra pathfinding, and an ordered list of `UNodeAction` components executed on arrival. A pawn queue ensures that if multiple NPCs arrive simultaneously, actions run serially rather than in parallel.",
            badges: [
                { text: "Actor",         cls: "badge-actor" },
                { text: "Blueprintable", cls: "badge-blueprintable" }
            ],
            body: [
                {
                    title: "Properties",
                    type:  "propertyTable",
                    rows: [
                        {
                            name: "AcceptanceRadius", type: "float", default: "50.0",
                            desc: "Distance in cm at which the pawn is considered to have reached this node. Passed directly to `MoveToActor`.",
                            meta: [{ text: "Path Node", cls: "meta-cat" }, { text: "BlueprintReadWrite", cls: "meta-bp" }]
                        },
                        {
                            name: "ConnectedNodes", type: "TArray<AAiPathNode*>", default: "—",
                            desc: "Adjacent nodes used by `UNodeGraphHelper` to build the pathfinding graph. Both directions must be manually wired for bidirectional traversal.",
                            meta: [{ text: "Path Node", cls: "meta-cat" }, { text: "BlueprintReadWrite", cls: "meta-bp" }]
                        },
                        {
                            name: "bIsGoalNode", type: "bool", default: "false",
                            desc: "Semantic flag — marks the node as a destination. Used by game logic (cinematics, spawn points, etc.); has no effect on movement itself.",
                            meta: [{ text: "Path Node", cls: "meta-cat" }, { text: "BlueprintReadWrite", cls: "meta-bp" }]
                        },
                        {
                            name: "bNodeBusy", type: "bool", default: "false",
                            desc: "Set to `true` while actions are executing. Arriving pawns are queued and will begin their action sequence when the node becomes free.",
                            meta: [{ text: "BlueprintReadOnly", cls: "meta-bp" }]
                        }
                    ]
                },
                {
                    title: "Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "OnArrived(APawn*, TFunction<void()>)",
                            desc: "Called by `UAiPathFollowComponent` when the pawn successfully reaches this node. Executes each valid `UNodeAction` sequentially via `ExecuteActionAsync`. Invokes the completion callback once all actions finish. If the node is busy, the call is queued.",
                            meta: [{ text: "C++ API", cls: "meta-prot" }]
                        }
                    ]
                },
                {
                    title: "Delegates",
                    type:  "delegateList",
                    delegates: [
                        {
                            sig:  "DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNodeReached, AAiPathNode*, Node, AActor*, ArrivedActor)",
                            desc: "Broadcast when an actor reaches this node. Exposed for Blueprint binding."
                        }
                    ]
                }
            ]
        },

        // ── AAiPatrolPath ──────────────────────────────────────────────────────
        {
            id:          "AAiPatrolPath",
            name:        "AAiPatrolPath",
            parents:     ["AActor"],
            navDot:      "dot-actor",
            description: "A container actor that holds an ordered list of `AAiPathNode` references and renders them as a spline in the editor for visual feedback. Does not drive movement itself — pass `PathPoints` to `UAiPathFollowComponent::FollowNodeRoute`.",
            badges: [
                { text: "Actor", cls: "badge-actor" }
            ],
            body: [
                {
                    title: "Properties",
                    type:  "propertyTable",
                    rows: [
                        {
                            name: "PathSpline", type: "USplineComponent*",
                            desc: "Visual spline component. Rebuilt automatically in `OnConstruction` whenever `PathPoints` changes in the editor.",
                            meta: [{ text: "Component", cls: "meta-cat" }, { text: "BlueprintReadOnly", cls: "meta-bp" }]
                        },
                        {
                            name: "PathPoints", type: "TArray<AAiPathNode*>",
                            desc: "Ordered array of waypoints defining the patrol route. Pass this array to `FollowNodeRoute` to begin traversal.",
                            meta: [{ text: "AI Path", cls: "meta-cat" }, { text: "BlueprintReadWrite", cls: "meta-bp" }]
                        }
                    ]
                },
                {
                    title: "Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "RebuildSpline()",
                            desc: "Updates the spline control points to match the current `PathPoints` array. Called automatically in `OnConstruction`."
                        }
                    ]
                }
            ]
        },

        // ── UAiPathFollowComponent ─────────────────────────────────────────────
        {
            id:          "UAiPathFollowComponent",
            name:        "UAiPathFollowComponent",
            parents:     ["UActorComponent"],
            navDot:      "dot-component",
            description: "Moves the owner pawn along an ordered array of `AAiPathNode` actors using the AI pathfinding system. Handles per-node arrival callbacks, retry logic for failed moves, NPC collision avoidance via timed yielding, and pause/resume of an in-progress route. Node advancement is deferred one tick after each arrival to avoid calling `MoveToActor` from inside a `PathFollowingComponent` callback.",
            badges: [
                { text: "ActorComponent",    cls: "badge-component" },
                { text: "BlueprintSpawnable", cls: "badge-blueprintable" }
            ],
            body: [
                {
                    title: "Public Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "FollowNodeRoute(const TArray<AAiPathNode*>&)",
                            desc: "Starts traversal from index 0 of the supplied route. Caches the pawn and controller, resets all counters, and begins movement immediately.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "NodeFollower", cls: "meta-cat" }]
                        },
                        {
                            sig:  "StopFollowingRoute()",
                            desc: "Cancels the active route, clears all timers, stops movement, and empties `NodeRoute`. The component returns to an idle state.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "NodeFollower", cls: "meta-cat" }]
                        },
                        {
                            sig:  "PausePathFollow()",
                            desc: "Suspends movement without clearing the route or node index. All timers are cancelled and `StopMovement` is called. The NPC will stand still until `ResumePathFollow` is called. Guards against double-pause.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "NodeFollower", cls: "meta-cat" }]
                        },
                        {
                            sig:  "ResumePathFollow()",
                            desc: "Resumes from the node that was the active target when `PausePathFollow` was called. No backtracking occurs; `CurrentNodeIndex` is unchanged.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "NodeFollower", cls: "meta-cat" }]
                        },
                        {
                            sig:  "IsFollowingRoute() : bool",
                            desc: "Returns `true` when the component is actively moving along a route. Returns `false` when stopped or paused.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "BlueprintPure", cls: "meta-pure" }, { text: "NodeFollower", cls: "meta-cat" }]
                        },
                        {
                            sig:  "IsPathFollowPaused() : bool",
                            desc: "Returns `true` specifically when the route is paused (not stopped). Use this to distinguish a temporary pause from a full stop.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "BlueprintPure", cls: "meta-pure" }, { text: "NodeFollower", cls: "meta-cat" }]
                        }
                    ]
                },
                {
                    title: "Configurable Properties",
                    type:  "propertyTable",
                    rows: [
                        {
                            name: "MaxMoveRetries", type: "int32", default: "3",
                            desc: "Number of times to retry a failed `MoveToActor` before skipping the node entirely.",
                            meta: [{ text: "NodeFollower", cls: "meta-cat" }]
                        },
                        {
                            name: "RetryDelay", type: "float", default: "1.0",
                            desc: "Seconds to wait between failed move retries.",
                            meta: [{ text: "NodeFollower", cls: "meta-cat" }]
                        },
                        {
                            name: "StuckCheckInterval", type: "float", default: "0.5",
                            desc: "How often (seconds) to sample the pawn's position to detect if it is stuck against another agent.",
                            meta: [{ text: "NodeFollower|Avoidance", cls: "meta-cat" }]
                        },
                        {
                            name: "StuckDistanceThreshold", type: "float", default: "25.0",
                            desc: "Minimum distance in cm the NPC must travel per `StuckCheckInterval` to not be considered stuck.",
                            meta: [{ text: "NodeFollower|Avoidance", cls: "meta-cat" }]
                        },
                        {
                            name: "YieldDuration", type: "float", default: "0.75",
                            desc: "Base pause time in seconds when a stuck condition is detected. The actual yield is randomised to `YieldDuration + rand(0, YieldDuration * 0.5)` so two NPCs jammed together yield for different durations, letting one get ahead.",
                            meta: [{ text: "NodeFollower|Avoidance", cls: "meta-cat" }]
                        }
                    ]
                }
            ]
        },

        // ── UDmtAiBehaviorComponent ────────────────────────────────────────────
        {
            id:          "UDmtAiBehaviorComponent",
            name:        "UDmtAiBehaviorComponent",
            parents:     ["UActorComponent"],
            navDot:      "dot-component",
            description: "The central orchestrator for NPC AI behaviour. Attaches to the pawn, caches the `UAIPerceptionComponent`, instantiates the configured script class, and routes perception updates to the active script. Also acts as the bridge between C++ perception events and Blueprint-level script logic.",
            badges: [
                { text: "ActorComponent",    cls: "badge-component" },
                { text: "BlueprintSpawnable", cls: "badge-blueprintable" }
            ],
            body: [
                {
                    title: "Public Properties",
                    type:  "propertyTable",
                    rows: [
                        {
                            name: "ScriptClass", type: "TSubclassOf<UDmtAiV2ScriptBase>",
                            desc: "The script class to instantiate on `BeginPlay`. Assign a Blueprint subclass of `UDmtAiV2ScriptBase` in the Details panel.",
                            meta: [{ text: "NPC Settings", cls: "meta-cat" }]
                        },
                        {
                            name: "bEnableDebugTrace", type: "bool",
                            desc: "Enables verbose debug output from the component and active script.",
                            meta: [{ text: "Debug", cls: "meta-cat" }]
                        },
                        {
                            name: "OnPerceptionTargetUpdated", type: "FOnPerceptionTargetUpdated",
                            desc: "Multicast delegate fired each time the AI perception system reports an actor update. Scripts bind to this in `Init()`.",
                            meta: [{ text: "BlueprintAssignable", cls: "meta-bp" }]
                        }
                    ]
                },
                {
                    title: "Public Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "StopScript(FString Reason)",
                            desc: "Instructs the active script to stop. The reason string is passed for logging or state cleanup.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "GetActiveScript() : UDmtAiV2ScriptBase*",
                            desc: "Returns the currently running script instance, or `null` if none.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "BlueprintPure", cls: "meta-pure" }]
                        },
                        {
                            sig:  "DestroyScript()",
                            desc: "Destroys and nulls the active script object.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "SetSightPeripheralVisionHalfAngle(float)",
                            desc: "Updates the peripheral vision half-angle of the perception component's sight config at runtime.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "IsDebugTraceEnabled() : bool",
                            desc: "Returns the value of `bEnableDebugTrace`.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "BlueprintPure", cls: "meta-pure" }]
                        }
                    ]
                },
                {
                    title: "Structures",
                    type:  "structBlock",
                    structs: [
                        {
                            name: "FCharacterAiData",
                            fields: [
                                { name: "FName Name",          desc: "Human-readable identifier for the character." },
                                { name: "FGameplayTag Faction", desc: "Gameplay tag representing the actor's faction (e.g. `Faction.Player`, `Faction.Guard`)." }
                            ]
                        }
                    ]
                }
            ]
        },

        // ── UDmtAiV2ScriptBase ────────────────────────────────────────────────
        {
            id:          "UDmtAiV2ScriptBase",
            name:        "UDmtAiV2ScriptBase",
            parents:     ["UObject"],
            navDot:      "dot-script",
            description: "Abstract base class for all NPC AI scripts. Defines the three-state machine (`PATROL`, `ALERT`, `ATTACK`), faction-bucketed perception maps, threat tracking via `FAiTrackedActorData`, and the full suite of BlueprintNativeEvent hooks that subclasses override to implement specific NPC behaviour.",
            badges: [
                { text: "UObject",       cls: "badge-object" },
                { text: "Blueprintable", cls: "badge-blueprintable" }
            ],
            body: [
                {
                    title: "Enumerations",
                    type:  "enumBlock",
                    enums: [
                        {
                            name: "ENPCAIState", type: "uint8",
                            values: [
                                { name: "PATROL", desc: "Default state. NPC follows patrol routes; low threat awareness." },
                                { name: "ALERT",  desc: "NPC is aware of a threat but has not confirmed LOS (Common) or has not selected a target (Simple)." },
                                { name: "ATTACK", desc: "NPC has a confirmed, reachable target and is engaging." }
                            ]
                        }
                    ]
                },
                {
                    title: "Structures",
                    type:  "structBlock",
                    structs: [
                        {
                            name: "FAiTrackedActorData",
                            fields: [
                                { name: "FName Name",                          desc: "Actor identifier." },
                                { name: "TWeakObjectPtr<AActor> TargetActor",  desc: "Weak reference — safe if actor is destroyed." },
                                { name: "bool bSighted",                       desc: "Whether the last stimulus was a sight stimulus." },
                                { name: "FVector LastSenseLocation",           desc: "World position of the last perception event for this actor." },
                                { name: "float LastStimulusTime",              desc: "Game time at last stimulus, used for timeout removal." },
                                { name: "float ThreatValue",                   desc: "Accumulated threat score. Higher values take attack priority." },
                                { name: "AddThreat / DeductThreat / ResetThreat", desc: "Helpers to modify `ThreatValue`." }
                            ]
                        }
                    ]
                },
                {
                    title: "Protected Properties",
                    type:  "propertyTable",
                    rows: [
                        {
                            name: "HostileFactions", type: "FGameplayTagContainer",
                            desc: "Actors with tags matching any entry here are treated as hostile.",
                            meta: [{ text: "NPC Settings", cls: "meta-cat" }]
                        },
                        {
                            name: "FriendlyFactions", type: "FGameplayTagContainer",
                            desc: "Actors matching these tags are treated as friendly.",
                            meta: [{ text: "NPC Settings", cls: "meta-cat" }]
                        },
                        {
                            name: "AiDisabledTags", type: "FGameplayTagContainer",
                            desc: "If the owner actor has any of these gameplay tags, `AiIsDisabled()` returns `true` and AI logic should be suppressed.",
                            meta: [{ text: "NPC Settings", cls: "meta-cat" }]
                        },
                        {
                            name: "HostileActorData / NeutralActorData / FriendlyActorData",
                            type: "TMap<AActor*, FAiTrackedActorData>",
                            desc: "Live perception maps. Populated and cleaned up by `UpdatePerceivedTrackedActorState` / `UpdateLostTrackedActorState` / `UpdateDataMaps`.",
                            meta: [{ text: "Perceived Actors", cls: "meta-cat" }]
                        },
                        {
                            name: "CurrentTarget", type: "AActor*",
                            desc: "The actor currently selected for engagement. Set via `SetCurrentTarget`."
                        }
                    ]
                },
                {
                    title: "Public Functions & Events",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "ScriptTick(float DeltaTime)",
                            desc: "Called every frame by the behavior component. Override to drive per-frame AI logic.",
                            meta: [{ text: "BlueprintNativeEvent", cls: "meta-native" }]
                        },
                        {
                            sig:  "BP_Init(UDmtAiBehaviorComponent*)",
                            desc: "Called once when the script is initialised. Override to cache references and start logic.",
                            meta: [{ text: "BlueprintNativeEvent", cls: "meta-native" }]
                        },
                        {
                            sig:  "OnHostileTargetPerceived(AActor*, FAIStimulus, FCharacterAiData)",
                            desc: "Fires when a hostile actor is first perceived or re-perceived.",
                            meta: [{ text: "BlueprintNativeEvent", cls: "meta-native" }]
                        },
                        {
                            sig:  "OnHostileTargetLost(AActor*, FAIStimulus, FCharacterAiData)",
                            desc: "Fires when a previously perceived hostile actor is lost.",
                            meta: [{ text: "BlueprintNativeEvent", cls: "meta-native" }]
                        },
                        {
                            sig:  "OnNeutralTargetPerceived / OnFriendlyTargetPerceived",
                            desc: "Equivalent events for neutral and friendly faction actors.",
                            meta: [{ text: "BlueprintNativeEvent", cls: "meta-native" }]
                        },
                        {
                            sig:  "GetCurrentAIState() : ENPCAIState",
                            desc: "Returns the current state.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "BlueprintPure", cls: "meta-pure" }]
                        },
                        {
                            sig:  "SetCurrentAIState(ENPCAIState)",
                            desc: "Transitions to a new state and fires the corresponding delegate.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "GetActiveHostileTargets() : TMap&",
                            desc: "Returns a reference to the hostile perception map.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }, { text: "BlueprintPure", cls: "meta-pure" }]
                        },
                        {
                            sig:  "SelectAttackTarget()",
                            desc: "Chooses the attack target from the hostile map. Default implementation selects by highest `ThreatValue`. Override to customise.",
                            meta: [{ text: "BlueprintNativeEvent", cls: "meta-native" }]
                        }
                    ]
                },
                {
                    title: "State Delegates",
                    type:  "delegateList",
                    delegates: [
                        { sig: "OnTransitionToAttackDelegate — FOnTransitionToAttack", desc: "Broadcast when transitioning into `ATTACK` state." },
                        { sig: "OnTransitionToAlertDelegate — FOnTransitionToAlert",   desc: "Broadcast when transitioning into `ALERT` state." },
                        { sig: "OnTransitionToPatrolDelegate — FOnTransitionToPatrol", desc: "Broadcast when transitioning into `PATROL` state." }
                    ]
                }
            ]
        },

        // ── UDmtAiScriptV2Common ───────────────────────────────────────────────
        {
            id:          "UDmtAiScriptV2Common",
            name:        "UDmtAiScriptV2Common",
            parents:     ["UDmtAiV2ScriptBase"],
            navDot:      "dot-script",
            description: "Full-featured perception script with line-of-sight validation on every target query, per-state vision cone angles, a configurable state-assessment cooldown, and automatic timeout removal of stale tracked actors. Recommended for most NPCs where combat fidelity matters.",
            badges: [
                { text: "UObject",       cls: "badge-object" },
                { text: "Blueprintable", cls: "badge-blueprintable" }
            ],
            body: [
                {
                    title: "Properties",
                    type:  "propertyTable",
                    rows: [
                        {
                            name: "PatrolStateSightVisionAngle", type: "float", default: "30.0°",
                            desc: "Half-angle of the peripheral vision cone while in PATROL. Narrower — the NPC is less alert.",
                            meta: [{ text: "Settings", cls: "meta-cat" }]
                        },
                        {
                            name: "AlertStateSightVisionAngle", type: "float", default: "60.0°",
                            desc: "Half-angle while in ALERT. Wider than patrol.",
                            meta: [{ text: "Settings", cls: "meta-cat" }]
                        },
                        {
                            name: "AttackStateSightVisionAngle", type: "float", default: "180.0°",
                            desc: "Full hemisphere when engaged — the NPC sees everywhere.",
                            meta: [{ text: "Settings", cls: "meta-cat" }]
                        },
                        {
                            name: "StateAssessmentCooldown", type: "float", default: "0.2",
                            desc: "Minimum seconds between `AssessAIState` evaluations. Prevents per-frame state thrashing.",
                            meta: [{ text: "AI State", cls: "meta-cat" }]
                        },
                        {
                            name: "ActorTrackingTimeout", type: "float", default: "15.0",
                            desc: "Seconds after last stimulus before a tracked actor is removed from the faction maps by `UpdateDataMaps`.",
                            meta: [{ text: "State Settings", cls: "meta-cat" }]
                        },
                        {
                            name: "IgnoreTargetTrackingTags", type: "FGameplayTagContainer",
                            desc: "Actors carrying any of these gameplay tags are skipped during target tracking updates.",
                            meta: [{ text: "GameplayTags", cls: "meta-cat" }]
                        },
                        {
                            name: "TargetTraceCollisionChannel", type: "ECollisionChannel", default: "ECC_Visibility",
                            desc: "Collision channel used for LOS line traces in `IsTargetInLOS`.",
                            meta: [{ text: "Settings", cls: "meta-cat" }]
                        }
                    ]
                },
                {
                    title: "Key Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "AssessAIState()",
                            desc: "Evaluates whether a state transition is warranted based on current perception maps and LOS validity. Gated by `StateAssessmentCooldown`."
                        },
                        {
                            sig:  "HasAnyAttackTargets() : bool",
                            desc: "Returns `true` if any hostile actor is in LOS. Requires a successful trace.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "HasAnyAlertTargets() : bool",
                            desc: "Returns `true` if any hostile actor is tracked, regardless of LOS.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "IsTargetInLOS(AActor*) : bool",
                            desc: "Performs a line trace from the NPC to the target on `TargetTraceCollisionChannel`."
                        },
                        {
                            sig:  "UpdateDataMaps()",
                            desc: "Removes entries from all faction maps whose `LastStimulusTime` exceeds `ActorTrackingTimeout`.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "GetYoungestAlertLocation() : FVector",
                            desc: "Returns the `LastSenseLocation` of the most recently perceived hostile actor.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        }
                    ]
                }
            ]
        },

        // ── UDmtAiScriptV2Simple ───────────────────────────────────────────────
        {
            id:          "UDmtAiScriptV2Simple",
            name:        "UDmtAiScriptV2Simple",
            parents:     ["UDmtAiV2ScriptBase"],
            navDot:      "dot-script",
            description: "Lightweight variant of `UDmtAiScriptV2Common` that skips line-of-sight validation. Targets are selected purely from the perception map without ray casting. Lower CPU overhead — suitable for background NPCs, crowds, or cases where LOS fidelity is not required.",
            badges: [
                { text: "UObject",       cls: "badge-object" },
                { text: "Blueprintable", cls: "badge-blueprintable" }
            ],
            body: [
                {
                    title: "",
                    type:  "calloutGroup",
                    callouts: [
                        {
                            type: "note",
                            text: "Exposes the same configuration properties as `UDmtAiScriptV2Common` (vision angles, timeout, ignore tags, collision channel) but `GetCurrentTarget` returns the best-threat actor without performing a trace. Use `Common` when NPCs must confirm visual contact before engaging."
                        }
                    ]
                }
            ]
        },

        // ── UAiPathScriptBase ──────────────────────────────────────────────────
        {
            id:          "UAiPathScriptBase",
            name:        "UAiPathScriptBase",
            parents:     ["UObject"],
            navDot:      "dot-script",
            description: "Lightweight base class for one-shot execution scripts tied to a patrol path. Subclass in Blueprint, implement `Execute(Actor)`, and call `FinishExecute(bool)` when done. The `OnScriptFinished` delegate notifies the caller.",
            badges: [
                { text: "UObject",       cls: "badge-object" },
                { text: "Blueprintable", cls: "badge-blueprintable" }
            ],
            body: [
                {
                    title: "Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "NativeExecute(AActor*)",
                            desc: "C++ entry point. Caches the world context and calls the Blueprint-implementable `Execute`.",
                            meta: [{ text: "C++ API", cls: "meta-prot" }]
                        },
                        {
                            sig:  "Execute(AActor*)",
                            desc: "Override in Blueprint to implement the script's logic.",
                            meta: [{ text: "BlueprintImplementableEvent", cls: "meta-impl" }]
                        },
                        {
                            sig:  "FinishExecute(bool bSuccess)",
                            desc: "Call from Blueprint when the script completes. Fires `OnScriptFinished`.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        }
                    ]
                }
            ]
        },

        // ── UNodeAction ────────────────────────────────────────────────────────
        {
            id:          "UNodeAction",
            name:        "UNodeAction",
            parents:     ["UActorComponent"],
            navDot:      "dot-object",
            description: "Abstract base for behaviours executed when an NPC arrives at an `AAiPathNode`. Add one or more `UNodeAction` subcomponents to a node actor; they run in component order when the node is arrived at. Override `BP_ExecuteAction` in Blueprint for custom logic, and call `OnFinished()` to advance the action chain.",
            badges: [
                { text: "ActorComponent", cls: "badge-component" },
                { text: "Blueprintable",  cls: "badge-blueprintable" }
            ],
            body: [
                {
                    title: "Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "CanExecute(APawn*) : bool",
                            desc: "Return `false` to skip this action for the given pawn. Default returns `true`.",
                            meta: [{ text: "BlueprintNativeEvent", cls: "meta-native" }]
                        },
                        {
                            sig:  "ExecuteActionAsync(APawn*, TFunction<void()>)",
                            desc: "Called by the node to start this action. Stores the completion callback and invokes `BP_ExecuteAction`.",
                            meta: [{ text: "C++ API", cls: "meta-prot" }]
                        },
                        {
                            sig:  "BP_ExecuteAction(APawn*)",
                            desc: "Override in Blueprint. Implement any logic here — latent nodes, timelines, montages, etc. **Must call `OnFinished()` when done** or the route will stall.",
                            meta: [{ text: "BlueprintNativeEvent", cls: "meta-native" }]
                        },
                        {
                            sig:  "OnFinished()",
                            desc: "Signal that this action is complete. Fires the stored callback, advancing the action chain to the next action or triggering node completion.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        }
                    ]
                }
            ],
            callouts: [
                {
                    type: "warn",
                    text: "Always call `OnFinished()` in every code path of `BP_ExecuteAction`. If it is never called, the NPC will stop permanently at the node with no error."
                }
            ]
        },

        // ── UNodeGraphHelper ───────────────────────────────────────────────────
        {
            id:          "UNodeGraphHelper",
            name:        "UNodeGraphHelper",
            parents:     ["UObject"],
            navDot:      "dot-object",
            description: "Static utility class providing Dijkstra pathfinding over the `AAiPathNode` graph defined by each node's `ConnectedNodes` array. Use `FindShortestPath` to build an ordered route array suitable for passing to `UAiPathFollowComponent::FollowNodeRoute`.",
            badges: [
                { text: "UObject", cls: "badge-object" }
            ],
            body: [
                {
                    title: "Static Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "FindShortestPath(AAiPathNode* From, AAiPathNode* To) : TArray<AAiPathNode*>",
                            desc: "Runs Dijkstra's algorithm on the node graph and returns the shortest ordered path from `From` to `To`. Returns an empty array if no path exists.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "FindNearestNode(AActor* Actor, FVector Origin) : AAiPathNode*",
                            desc: "Finds the `AAiPathNode` in the world closest to `Origin`. Useful for snapping an NPC's start point to the nearest node before calling `FindShortestPath`.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        }
                    ]
                }
            ]
        },

        // ── IDmtAiInterfaceV2 ──────────────────────────────────────────────────
        {
            id:          "IDmtAiInterfaceV2",
            name:        "IDmtAiInterfaceV2",
            parents:     ["UInterface"],
            navDot:      "dot-interface",
            description: "Implement on any actor that participates in the AI perception and targeting system. The script queries this interface to obtain faction tags, character metadata, and animation event routing. Actors without this interface cannot be classified by the faction system.",
            badges: [
                { text: "Interface", cls: "badge-interface-b" }
            ],
            body: [
                {
                    title: "Interface Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "GetFaction() : FGameplayTag",
                            desc: "Return the actor's faction tag. Used by scripts to determine routing into hostile, neutral, or friendly maps.",
                            meta: [{ text: "BlueprintImplementableEvent", cls: "meta-impl" }]
                        },
                        {
                            sig:  "GetCharacterAiData() : FCharacterAiData",
                            desc: "Return the actor's name and faction as a `FCharacterAiData` struct.",
                            meta: [{ text: "BlueprintImplementableEvent", cls: "meta-impl" }]
                        },
                        {
                            sig:  "GetActorGameplayTags() : FGameplayTagContainer",
                            desc: "Return all gameplay tags owned by this actor. Used by `AiIsDisabled()` to check `AiDisabledTags`.",
                            meta: [{ text: "BlueprintImplementableEvent", cls: "meta-impl" }]
                        },
                        {
                            sig:  "GetYoungestAlertLocation() : FVector",
                            desc: "Return the most recent alert world position for this actor.",
                            meta: [{ text: "BlueprintImplementableEvent", cls: "meta-impl" }]
                        },
                        {
                            sig:  "GetPatrolObject() : UObject*",
                            desc: "Return the actor's patrol data object (e.g. an `AAiPatrolPath`).",
                            meta: [{ text: "BlueprintImplementableEvent", cls: "meta-impl" }]
                        },
                        {
                            sig:  "AiAnimationEvent(uint8 Key)",
                            desc: "Trigger an animation event by numeric key. The actor is responsible for mapping keys to montages.",
                            meta: [{ text: "BlueprintImplementableEvent", cls: "meta-impl" }]
                        },
                        {
                            sig:  "GetAiAnimationFromEvent(uint8, UAnimMontage*&, USkeletalMeshComponent*&)",
                            desc: "Retrieve the montage and target skeletal mesh for an animation event key.",
                            meta: [{ text: "BlueprintImplementableEvent", cls: "meta-impl" }]
                        }
                    ]
                }
            ]
        },

        // ── IDmtAiPatrolInterface ──────────────────────────────────────────────
        {
            id:          "IDmtAiPatrolInterface",
            name:        "IDmtAiPatrolInterface",
            parents:     ["UInterface"],
            navDot:      "dot-interface",
            description: "Implement on actors that want to receive a notification each time they arrive at a patrol node. Useful for triggering Blueprint logic (animations, dialogue, state changes) directly on the character rather than through a node action.",
            badges: [
                { text: "Interface", cls: "badge-interface-b" }
            ],
            body: [
                {
                    title: "Interface Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "ArrivedAtPatrolNode(AAiPathNode* Node)",
                            desc: "Called by the path follow system when the actor reaches a node. The node reference is provided for context.",
                            meta: [{ text: "BlueprintImplementableEvent", cls: "meta-impl" }]
                        }
                    ]
                }
            ]
        },

        // ── UAiFunctionLibrary ─────────────────────────────────────────────────
        {
            id:          "UAiFunctionLibrary",
            name:        "UAiFunctionLibrary",
            parents:     ["UBlueprintFunctionLibrary"],
            navDot:      "dot-library",
            description: "Global static helpers for accessing AI components and scripts from any Blueprint without needing a direct object reference.",
            badges: [
                { text: "BPFunctionLibrary", cls: "badge-library" }
            ],
            body: [
                {
                    title: "Static Functions",
                    type:  "functionTable",
                    rows: [
                        {
                            sig:  "GetDmtAiBehaviorComponent(AActor*, UDmtAiBehaviorComponent*&) : bool",
                            desc: "Retrieves the `UDmtAiBehaviorComponent` attached to the given actor. Returns `false` and a null out-parameter if not found.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "GetActorAiScript(AActor*, UDmtAiV2ScriptBase*&) : bool",
                            desc: "Retrieves the active script from the actor's behavior component. Returns `false` if the actor has no behavior component or no active script.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        },
                        {
                            sig:  "GetDmtAiBehaviorTarget(AActor*, AActor*&) : bool",
                            desc: "Retrieves the current attack target from the actor's active script via `GetCurrentTarget`.",
                            meta: [{ text: "BlueprintCallable", cls: "meta-bp" }]
                        }
                    ]
                }
            ]
        }

    ], // end classes

    // ── DELEGATES REFERENCE ───────────────────────────────────────────────────
    delegatesRef: [
        {
            sig:  "FOnNodeReached(AAiPathNode* Node, AActor* ArrivedActor)",
            desc: "Declared on `AAiPathNode`. Broadcast when an actor arrives at the node."
        },
        {
            sig:  "FOnPerceptionTargetUpdated(AActor* Actor, FAIStimulus Stimulus)",
            desc: "Declared on `UDmtAiBehaviorComponent`. Forwarded from `UAIPerceptionComponent::OnTargetPerceptionUpdated`. Scripts bind to this in `Init()`."
        },
        {
            sig:  "FOnPathScriptFinished()",
            desc: "Declared on `UAiPathScriptBase`. Broadcast when `FinishExecute` is called."
        },
        {
            sig:  "FOnTransitionToAttack()",
            desc: "Declared on `UDmtAiV2ScriptBase`. Broadcast when `ENPCAIState` transitions to `ATTACK`."
        },
        {
            sig:  "FOnTransitionToAlert()",
            desc: "Declared on `UDmtAiV2ScriptBase`. Broadcast when `ENPCAIState` transitions to `ALERT`."
        },
        {
            sig:  "FOnTransitionToPatrol()",
            desc: "Declared on `UDmtAiV2ScriptBase`. Broadcast when `ENPCAIState` transitions to `PATROL`."
        }
    ]

}; // end PLUGIN_DATA
