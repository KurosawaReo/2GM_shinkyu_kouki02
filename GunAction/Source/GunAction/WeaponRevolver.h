/*
   - AWeaponRevolver -
   武器クラス「短銃」

   [銃の主なコンポーネント]
   Root
   └RevolverMain
	 └Muzzle
   　└Bullet_1
   　　└Shell_1
   　└Bullet_2
   　　└Shell_2
   　└Bullet_3
   　　└Shell_3
   　└Bullet_4
   　　└Shell_4
   　└Bullet_5
   　　└Shell_5
   　└Bullet_6
   　　└Shell_6
   └BoxCollision
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponRevolver.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UBoxComponent;
class UAudioComponent;
class UParticleSystemComponent;
class USceneComponent;

UCLASS()
class GUNACTION_API AWeaponRevolver : public AActor
{
	GENERATED_BODY()
	
//▼ ===== 変数 ===== ▼.
public:	
	/** ルートコンポーネント */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	/** リボルバー本体のメッシュ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components")
	TObjectPtr<USkeletalMeshComponent> RevolverMain;
	/** マズル(銃口)のメッシュ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components")
	TObjectPtr<UStaticMeshComponent> Muzzle;
	/** 当たり判定(box) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components")
	TObjectPtr<UBoxComponent> BoxCollision;

	/** 弾丸 & シェル */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Bullet_1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Shell_1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Bullet_2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Shell_2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Bullet_3;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Shell_3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Bullet_4;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Shell_4;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Bullet_5;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Shell_5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Bullet_6;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Components|Bullet")
	TObjectPtr<UStaticMeshComponent> Shell_6;

	/** シリンダーチャンバー開放音声 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Cylinder_Chamber_Open_Cue;

	/** シリンダーチャンバー閉鎖音声 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Cylinder_Chamber_Close_Cue;

	/** 発砲音声 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_01_Cue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_02_Cue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_03_Cue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_04_Cue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Audio")
	TObjectPtr<UAudioComponent> S_Revolver_Shot_05_Cue;

	/** マズルフラッシュ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Muzzle")
	TObjectPtr<UParticleSystemComponent> PS_Muzzleflash_Revolver;
	
	/** 銃の射撃アニメーション */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MyProperty|Animation")
	class UAnimMontage* FireAnimMontage;

	/** 銃のリロードアニメーション */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MyProperty|Animation")
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
	AWeaponRevolver();

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
