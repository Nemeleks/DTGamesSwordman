// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Weapons/Weapon.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	CharacterMesh->SetupAttachment(RootComponent);
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(CharacterMesh);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	
	WeaponConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("SwordConstraint"));
	WeaponConstraint->AttachToComponent(CharacterMesh,FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	LoadFromJason();

	Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, WeaponConstraint->GetComponentLocation(), WeaponConstraint->GetComponentRotation());

	WeaponConstraint->ConstraintActor2 = Weapon;
	WeaponConstraint->SetConstrainedComponents(CharacterMesh, "hand_r", Weapon->GetMeshComponent(), NAME_None);
	WeaponConstraint->InitComponentConstraint();

	Weapon->GetMeshComponent()->OnComponentHit.AddDynamic(this, &APlayerCharacter::OnHit);

}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UE_LOG(LogTemp, Warning, TEXT("WBS = %f"), WeaponBounceStr);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Attack);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnAround", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

}

void APlayerCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	 if (OtherActor != this && bIsAttacking)
	 {
		const FVector HitNormal = Hit.ImpactNormal;
		const FVector WeaponMoveDirection = Weapon->GetActorForwardVector();
		const FVector AttackReflectionVector = -FMath::GetReflectionVector(WeaponMoveDirection, HitNormal);

	 	Weapon->GetMeshComponent()->AddImpulse(AttackReflectionVector*WeaponBounceStr);
	 	
		SetPhysicsArm(true);
	 	
		bIsAttacking = false;

		GetWorld()->GetTimerManager().SetTimer(DisablePhysicsArmTimerHandle,this, &APlayerCharacter::DisablePhysicsArm, DisablePhysicsArmRate,false,DisablePhysicsArmDelay);
		
	 }
}

void APlayerCharacter::SetPhysicsArm(bool isPhysics)
{
	CharacterMesh->GetBodyInstance(TEXT("hand_r"))->SetInstanceSimulatePhysics(isPhysics);
	CharacterMesh->GetBodyInstance(TEXT("lowerarm_r"))->SetInstanceSimulatePhysics(isPhysics);
	CharacterMesh->GetBodyInstance(TEXT("upperarm_r"))->SetInstanceSimulatePhysics(isPhysics);
}

void APlayerCharacter::DisablePhysicsArm()
{
	SetPhysicsArm(false);
	GetWorld()->GetTimerManager().ClearTimer(DisablePhysicsArmTimerHandle);
}



void APlayerCharacter::LoadFromJason()
{
	const FString FileName = "MannequinParams.msg";
	const FString PathToContent = FPaths::ProjectContentDir()+"/Json/PlayerStats/" + FileName;
	FString JsonString;

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*PathToContent))
	{
		return;
	}
	FFileHelper::LoadFileToString(JsonString, *PathToContent);

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
	TSharedPtr<FJsonObject> JSonObject = MakeShareable(new FJsonObject());
	FJsonSerializer::Deserialize(JsonReader, JSonObject);

	TSharedPtr<FJsonObject> JsonStats = JSonObject->GetObjectField("player");

	WeaponBounceStr = JsonStats->GetNumberField("WeaponBounceStr");
}

void APlayerCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::Attack()
{
	if (!bIsAttacking)
	{
		bIsAttacking = true;
		GetWorld()->GetTimerManager().SetTimer(StopAttackAnimTimerHandle, this, &APlayerCharacter::StopAttackAnim, StopAttackAnimRate, false, StopAttackAnimDelay);
	}

}

void APlayerCharacter::StopAttackAnim()
{
	
	if (bIsAttacking)
	{
		bIsAttacking = false;
	}
	GetWorld()->GetTimerManager().ClearTimer(StopAttackAnimTimerHandle);
}