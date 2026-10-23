#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "NightSchoolArt.generated.h"

class UFont;
class UMaterialInstanceDynamic;
class UStaticMesh;
class UTexture2D;

/** 世界空间里显示运行时贴图。不依赖编辑器导入。 */
UCLASS()
class STUDENT_API UNightSchoolSpriteWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTexture(UTexture2D* InTexture);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TObjectPtr<UTexture2D> Texture;

	FSlateBrush Brush;
};

/** 引擎基础形体、运行时贴图和中文字体。贴图是代码绘制的，不另下资源。 */
namespace NightSchoolArt
{
	UStaticMesh* Cube();
	UStaticMesh* Sphere();
	UStaticMesh* Cylinder();
	UStaticMesh* Cone();
	UMaterialInstanceDynamic* MakeColor(UObject* Outer, const FLinearColor& Color);
	UFont* GetRuntimeFont();
	FSlateFontInfo FontInfo(float Size);

	UTexture2D* MakeMirror(UObject* Outer);
	UTexture2D* MakeScoreSheet(UObject* Outer);
	UTexture2D* CreateTexture(UObject* Outer, int32 Width, int32 Height, const TArray<FColor>& Pixels);
}
