// Copyright Dalton Lima 2020

#include "OpenDoor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

#define GAMENAME_DEBUG 1
//#define DO_CHECK 1

#if GAMENAME_DEBUG
    #define GAMENAME_CHECK(expr) { if(!(expr)) FDebug::AssertFailed( #expr, __FILE__, __LINE__ ); CA_ASSUME(expr); }
#else
    #define GAMENAME_CHECK(expr)
#endif

UOpenDoor::UOpenDoor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UOpenDoor::BeginPlay()
{
	Super::BeginPlay();

	GAMENAME_CHECK(YawCurve);

	/*
	 * check() is a runtime assert that is compiled out in shipping and test builds (unless USE_CHECKS_IN_SHIPPING is 1).
     * checkSlow() behaves like a check() in debug and is compiled out otherwise (for safety checks that are good but rare / too slow to use in normal builds)
     * ensure() is for validating conditions that can still be handled even if they fail. It evaluates the expression and traps if the debugger is attached
     * when it fails, but the code will keep executing afterwards. The expression itself is never compiled out
     * (so you can write things like if (ensure(Foo != null)) { do stuff with Foo; }).
	 */
	
	//check(PressurePlate != nullptr);
	//checkf(PressurePlate, TEXT("ATriggerVolume PressurePlate is null in %s"), *GetOwner()->GetName());
    //ensure()>
    ensureMsgf(PressurePlate, TEXT("ATriggerVolume PressurePlate is null in %s"), *GetOwner()->GetName());

    //checkf(WasDestroyed, TEXT("Failed to destroy Actor %s (%s)"), *Actor->GetClass()->GetName(), *Actor->GetActorLabel());

    //UE_LOG(LogTemp, Warning, TEXT("[OpenDoor] BeginPlay @ %s"), *GetOwner()->GetName());

	APawn* PawnController = GetWorld()->GetFirstPlayerController()->GetPawn();

	InitialRotation = GetOwner()->GetActorRotation();

	ElapsedTime = 0;
	bIsOpen = false;
	FindAudioComponent();
}

void UOpenDoor::FindAudioComponent()
{
	AudioComponent = GetOwner()->FindComponentByClass<UAudioComponent>();
	if (!AudioComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[OpenDoor] Missing AudioComponent! %s"), *GetOwner()->GetName());
	}
}

void UOpenDoor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	

	if (TotalMassOfActors() > MassToOpenDoor)
	{
		OpenDoor(DeltaTime);
	}
	else
	{
		CloseDoor(DeltaTime);
	}

}

void UOpenDoor::OpenDoor(const float DeltaTime)
{
	if (ElapsedTime > OpeningDuration)
	{
		//bIsClosed = false;
		//bIsOpen = true;
		DoorLastOpened = GetWorld()->GetTimeSeconds();
		//ElapsedTime = OpeningDuration;
		return;
	}

	ElapsedTime += DeltaTime;

	FRotator CurrentRotation = GetOwner()->GetActorRotation();

	GAMENAME_CHECK(YawCurve);

	float CurrentYaw = YawCurve->GetFloatValue(ElapsedTime / OpeningDuration);
	CurrentRotation.Yaw = InitialRotation.Yaw + CurrentYaw;
	GetOwner()->SetActorRelativeRotation(CurrentRotation);

	if (!AudioComponent) { return; }
	if (!bOpenDoorSoundPlayed)
	{
		AudioComponent->Play();
		bOpenDoorSoundPlayed = true;
		bCloseDoorSoundPlayed = false;
	}
}

void UOpenDoor::CloseDoor(const float DeltaTime)
{
	//bIsOpen = false;
	if (DoorLastOpened + DoorCloseDelayInSeconds > GetWorld()->GetTimeSeconds())
	{
	    return;
	}

	GAMENAME_CHECK(YawCurve);

	ElapsedTime -= DeltaTime;

	if (ElapsedTime < 0)
	{
		//bIsClosed = true;
		ElapsedTime = 0.f;
	}

	FRotator CurrentRotation = GetOwner()->GetActorRotation();

	float CurrentYaw = YawCurve->GetFloatValue(ElapsedTime / OpeningDuration);
	CurrentRotation.Yaw = InitialRotation.Yaw + CurrentYaw;
	GetOwner()->SetActorRelativeRotation(CurrentRotation);

	if (!AudioComponent) { return; }
	if (!bCloseDoorSoundPlayed)
	{
		AudioComponent->Play();
		bCloseDoorSoundPlayed = true;
		bOpenDoorSoundPlayed = false;
	}
}

float UOpenDoor::TotalMassOfActors() const
{
	float TotalMass = 0.f;
	TArray<AActor*> OverlappingActors;

	if (!PressurePlate) { return TotalMass; }
	PressurePlate->GetOverlappingActors(OUT OverlappingActors);

	for (AActor* OverlappingActor : OverlappingActors)
	{
		TotalMass += OverlappingActor->FindComponentByClass<UPrimitiveComponent>()->GetMass();
	}
	//UE_LOG(LogTemp, Warning, TEXT("%f on pressure plate"), TotalMass)
	return TotalMass;
}
