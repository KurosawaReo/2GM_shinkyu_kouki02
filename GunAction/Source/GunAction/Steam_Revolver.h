// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Steam_Revolver.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UBoxComponent;
class UAudioComponent;
class UParticleSystemComponent;
class USceneComponent;

UCLASS()
class GUNACTION_API ASteam_Revolver : public AActor
{
	GENERATED_BODY()
	
//▼ ===== 変数 ===== ▼.
public:	
	/** ルートシーンコンポーネント */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	/** リボルバーの本体スケルタルメッシュ */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> Steam_Revolver;

	/** 弾丸 & シェル */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Bullet_1;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Shell_1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Bullet_2;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Shell_2;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Bullet_3;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Shell_3;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Bullet_4;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Shell_4;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Bullet_5;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Shell_5;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Bullet_6;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Ammunition")
	TObjectPtr<UStaticMeshComponent> Shell_6;

	/** マズル（銃口）スタティックメッシュ */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Muzzle")
	TObjectPtr<UStaticMeshComponent> Muzzle;

	/** コリジョン用ボックス */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> Box;

	/** シリンダーチャンバー開放音声 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Cylinder_Chamber_Open_Cue;

	/** シリンダーチャンバー閉鎖音声 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Cylinder_Chamber_Close_Cue;

	/** 発砲音声 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_01_Cue;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_02_Cue;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_03_Cue;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_04_Cue;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_05_Cue;

	/** マズルフラッシュ */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components|Muzzle")
	TObjectPtr<UParticleSystemComponent> PS_Muzzleflash_Revolver;
	
	/** 銃の射撃アニメーション */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* FireAnimMontage;

	/** 銃のリロードアニメーション */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* ReloadAnimMontage;

//▼ ===== 関数 ===== ▼.
private:
	/* 全てのコンポーネントのコリジョンを無効化 */
	void DisableAllCollisions();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Sets default values for this actor's properties
	ASteam_Revolver();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/* 銃のメッシュを無効化 */
	void DisableGunMesh();

	/* 射撃アニメーションを再生 */
	UFUNCTION(BlueprintCallable, Category = "Gun")
	void PlayFireAnimation();

	/* リロードアニメーションを再生 */
	UFUNCTION(BlueprintCallable, Category = "Gun")
	void PlayReloadAnimation();
};
