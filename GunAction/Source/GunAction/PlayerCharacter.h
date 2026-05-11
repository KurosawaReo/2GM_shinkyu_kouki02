/*
   - PlayerCharacter -
   共通のCharacterクラスから派生したプレイヤークラス.

   [クラス構成]
   CharacterBase
   └PlayerCharacter ←今ここ
   └EnemyCharacter
*/
#pragma once
#include "CoreMinimal.h"

//他class.
#include "CharacterBase.h" //親クラス.
#include "WeaponRevolver.h"

#include "PlayerCharacter.generated.h"

class UCrosshairWidget;
class USpringArmComponent;
class UCameraComponent;

/// <summary>
/// プレイヤークラス.
/// </summary>
UCLASS()
class GUNACTION_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()

//▼ ===== 変数 ===== ▼.
public:

#pragma region "カメラ"
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class UCameraComponent* FollowCamera;
#pragma endregion

#pragma region "クロスヘア"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|Crosshair")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|Crosshair")
	UCrosshairWidget* CrosshairWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|Crosshair")
	bool IsShowCrosshair = true; //クロスヘア表示,非表示切り替え用.
#pragma endregion

//▼ ===== 関数 ===== ▼.
protected:

#pragma region "基本処理"
	APlayerCharacter();						//コンストラクタ.
	void BeginPlay() override;				//召喚した瞬間に実行.
	void Tick(float DeltaTime) override;	//常に実行.
#pragma endregion

public:
#pragma region "入力処理"
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//入力バインド.
	void Input(UInputComponent* PlayerInputComponent);
#pragma endregion

#pragma region "ジャンプ"
	void Jump()        override; //ジャンプ開始.
	void StopJumping() override; //ジャンプ終了.
#pragma endregion

#pragma region "移動"
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnTurnRate(float Rate);
	void OnLookUpRate(float Rate);
#pragma endregion

#pragma region "射撃"
	void OnFire();				    	//射撃処理.
	void CrosshairWidgetExe() override;	//クロスヘアエフェクト実行.
#pragma endregion

#pragma region "ダメージ・死亡"
	void OnBulletHit() override;	//弾が当たったら実行される.
	void Death()       override;	//死亡処理.
#pragma endregion

#pragma region "UI"
	void InitUI();
#pragma endregion
};