/*
   - PlayerManager -
   共通のCharacterクラスから派生したプレイヤークラス.

   [クラス構成]
   CharacterBase
   └PlayerManager ←今ここ
   └EnemyManager
*/
#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h" //親クラス.
#include "Steam_Revolver.h"
#include "PlayerManager.generated.h"

class UCrosshairWidget;
class USpringArmComponent;
class UCameraComponent;

/// <summary>
/// プレイヤークラス.
/// </summary>
UCLASS()
class GUNACTION_API APlayerManager : public ACharacterBase
{ 
	GENERATED_BODY()

	//▼ ===== 変数 ===== ▼.
public:
	//クロスヘア.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|UI")
	UCrosshairWidget* CrosshairWidget;

	//カメラコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class UCameraComponent* FollowCamera;

	// マズルソケットの名前
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Gun|Muzzle")
	FName MuzzleSocketName = FName("Muzzle"); // スケルタルメッシュ内のソケット名

	//プレイヤーの腕処理.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|IK")
	bool bEnbLeArmIK = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|IK")
	FVector RightHandIKTarget = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|IK")
	float RightHandIKAlpha = 0.0f;

	//▼ ===== 関数 ===== ▼.
protected:
	//コンストラクタ.
	APlayerManager();

#pragma region "ライフサイクル"
	//召喚した瞬間.
	void BeginPlay() override;
	//常に実行.
	void Tick(float DeltaTime) override;
#pragma endregion

#pragma region "入力処理"
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Input(UInputComponent* PlayerInputComponent);
#pragma endregion

#pragma region "移動"
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void StartSprint();
	void StopSprint();
#pragma endregion

#pragma region "銃システム"
private:

	// マズル位置を取得する関数
	FVector GetMuzzleLocation() const;

	// マズル方向を取得する関数
	FRotator GetMuzzleRotation() const;

	//インデックス初期化.
	void InitializeArmIK();

	//クロスヘアと高さ.
	void UpdateArmIK();


#pragma endregion

#pragma region "カメラ"
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector  GetCameraVector(FString dir) const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	FVector  GetCameraLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	FRotator GetCameraRotation() const;
#pragma endregion

#pragma region "UI"
	void InitializeUI();
#pragma endregion
	
#pragma region "射撃"
	void ShotBullet() override; //override
	void SaveConfig();
#pragma endregion

public:
#pragma region "ダメージ処理"
	void OnBulletHit() override; //弾が当たったら実行される.
	void Die()         override; //死亡処理.
#pragma endregion
};
