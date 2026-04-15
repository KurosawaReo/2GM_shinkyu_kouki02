// Sword.cpp

#include "Sword.h"
#include "Components/StaticMeshComponent.h"

ASword::ASword()
{
	PrimaryActorTick.bCanEverTick = false;

	// ルートコンポーネント
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// スタティックメッシュコンポーネントを作成
	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	SwordMesh->SetupAttachment(RootComponent);

	// デフォルト設定
	SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASword::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Sword spawned and equipped!"));
}