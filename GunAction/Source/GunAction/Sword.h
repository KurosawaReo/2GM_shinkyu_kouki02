// Sword.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Sword.generated.h"

/// <summary>
/// 剣クラス.
/// プレイヤーが装備する武器.
/// 既存のスタティックメッシュを使用する.
/// </summary>
UCLASS()
class GUNACTION_API ASword : public AActor
{
	GENERATED_BODY()

public:
	ASword();

	virtual void BeginPlay() override;

	// スタティックメッシュコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sword")
	class UStaticMeshComponent* SwordMesh;
};