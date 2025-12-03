// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlyerCharacter.generated.h"

class UCrosshairWidget;
class USpringArmComponent;
class UCameraComponent;
class ABulletBase;


UCLASS()
class GUNACTION_API APlyerCharacter : public ACharacter
{
	GENERATED_BODY()

//▼変数.
public:	
	//クロスヘアWidgets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCrosshairWidget>CrosshairWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UCrosshairWidget* CrosshairWidget;

	//カメラコンポーネント.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;

	// 弾クラスの参照（Blueprintで設定可能）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bullet)
	TSubclassOf<AActor> BulletClass;

	//アニメーション関連.
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation);
	//class  UAnimMontage* IdleAnimMontage;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation);
	//class  UAnimMontage* MoveAnimMontage;

	//移動パラメーター.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float BaseTurnRate = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float BaseLookUpRate = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float WalkSpeed = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float RunSpeed = 800.0f;

	// 弾の発射距離（エディタで調整可能）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	float BulletTargetDistance = 10000.0f; // 100メートル先

	//インスペクターに表示する方法.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsSprinting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsJumping;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	double CurrentSpeed;
	
//▼関数.
public:
	#pragma region "コンストラクタ".
	APlyerCharacter();
	#pragma endregion

protected:
	#pragma region "ライフサイクル".
	//召喚した瞬間.
	virtual void BeginPlay() override;
	//常に実行.
	virtual void Tick(float DeltaTime) override;
	#pragma endregion

	#pragma region "入力処理".
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Input(UInputComponent* PlayerInputComponent);
	#pragma endregion

	#pragma region "カメラ".
	// カメラの向きを取得する関数.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector  GetCameraVector(FString dir) const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector  GetCameraLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	FRotator GetCameraRotation() const;
	#pragma endregion

	#pragma region "移動".
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void StartSprint();
	void StopSprint();
	//アニメーションを更新.
	void UpdateAnimationState();
	#pragma endregion

	#pragma region "UI".
	void InitializeUI();;
	#pragma endregion

	#pragma region "射撃".
	// カメラの前方向 × 距離の位置を計算.
	void CalculateAndShot();
	//弾発射処理.
	void ShotBullet();
	// カメラ前方の目標地点を計算する関数.
	UFUNCTION(BlueprintCallable, Category = "Bullet")
	FVector CalculateTargetPosition(float Distance) const;
	#pragma endregion
};
