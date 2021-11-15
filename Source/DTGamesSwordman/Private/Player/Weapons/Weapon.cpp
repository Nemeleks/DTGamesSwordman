// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Weapons/Weapon.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacter.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMeshComponent=CreateDefaultSubobject<UStaticMeshComponent>("WeaponMesh");
	RootComponent = WeaponMeshComponent;

	OnWeaponHit = CreateDefaultSubobject<USoundBase>(TEXT("OnWeaponHit"));
	
	WeaponMeshComponent->OnComponentHit.AddDynamic(this, &AWeapon::PlayHitEffects);

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::PlayHitEffects(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (AttackHitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackHitEffect, Hit.ImpactPoint);
	}
	if (OnWeaponHit)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), OnWeaponHit, GetActorLocation());
	}
}

