#include "NightSchool/NightSchoolSlate.h"

#include "NightSchool/NightSchoolArt.h"
#include "NightSchool/NightSchoolCharacter.h"
#include "NightSchool/NightSchoolDirector.h"
#include "NightSchool/NightSchoolProp.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

class SNightSchoolHud : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNightSchoolHud) {}
		SLATE_ARGUMENT(ANightSchoolCharacter*, Character)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		Character = InArgs._Character;
		const FSlateFontInfo TitleFont = NightSchoolArt::FontInfo(28.f);
		const FSlateFontInfo BodyFont = NightSchoolArt::FontInfo(18.f);
		const FSlateFontInfo SmallFont = NightSchoolArt::FontInfo(15.f);
		const FLinearColor Paper(0.93f, 0.86f, 0.72f);
		const FLinearColor Dim(0.78f, 0.74f, 0.66f);

		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(32.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(TitleText, STextBlock)
					.Font(TitleFont)
					.ColorAndOpacity(Paper)
					.Text(FText::FromString(TEXT("回课")))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
				[
					SAssignNew(StatsText, STextBlock)
					.Font(SmallFont)
					.ColorAndOpacity(Dim)
					.AutoWrapText(true)
					.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
				[
					SAssignNew(ObjectiveText, STextBlock)
					.Font(BodyFont)
					.ColorAndOpacity(Paper)
					.AutoWrapText(true)
					.WrapTextAt(520.f)
					.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
				[
					SAssignNew(StatusText, STextBlock)
					.Font(SmallFont)
					.ColorAndOpacity(FLinearColor(0.86f, 0.45f, 0.38f))
					.AutoWrapText(true)
					.WrapTextAt(520.f)
					.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(Crosshair, STextBlock)
				.Font(BodyFont)
				.ColorAndOpacity(FLinearColor(0.9f, 0.82f, 0.7f, 0.85f))
				.Text(FText::FromString(TEXT("·")))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(24.f, 24.f, 24.f, 48.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(PromptText, STextBlock)
					.Font(BodyFont)
					.ColorAndOpacity(Paper)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Font(SmallFont)
					.ColorAndOpacity(FLinearColor(0.62f, 0.58f, 0.5f))
					.Justification(ETextJustify::Center)
					.Text(FText::FromString(TEXT("WASD 移动    鼠标 环顾    Shift 跑    Ctrl 蹲    E 交互    F 手电    Q 合上")))
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(40.f)
			[
				SAssignNew(NoteBorder, SBorder)
				.Visibility(EVisibility::Collapsed)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(FLinearColor(0.03f, 0.025f, 0.02f, 0.94f))
				.Padding(FMargin(36.f))
				[
					SNew(SBox)
					.WidthOverride(760.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
						[
							SAssignNew(NoteTitle, STextBlock)
							.Font(TitleFont)
							.ColorAndOpacity(FLinearColor(0.86f, 0.28f, 0.22f))
							.AutoWrapText(true)
							.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(NoteBody, STextBlock)
							.Font(BodyFont)
							.ColorAndOpacity(Paper)
							.AutoWrapText(true)
							.WrapTextAt(700.f)
							.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Font(SmallFont)
							.ColorAndOpacity(Dim)
							.Text(FText::FromString(TEXT("按 Q 把纸放回去")))
						]
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(EndingBorder, SBorder)
				.Visibility(EVisibility::Collapsed)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(FLinearColor(0.01f, 0.008f, 0.008f, 0.96f))
				.Padding(FMargin(80.f, 64.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 18.f)
					[
						SAssignNew(EndingTitle, STextBlock)
						.Font(TitleFont)
						.ColorAndOpacity(FLinearColor(0.9f, 0.32f, 0.24f))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(860.f)
						[
							SAssignNew(EndingBody, STextBlock)
							.Font(BodyFont)
							.ColorAndOpacity(Paper)
							.AutoWrapText(true)
							.WrapTextAt(820.f)
							.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
							.Justification(ETextJustify::Left)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 20.f, 0.f, 8.f)
					[
						SNew(STextBlock)
						.Font(SmallFont)
						.ColorAndOpacity(Dim)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(TEXT("按 R 再活一次这个除夕")))
					]
				]
			]
		];

		SetVisibility(EVisibility::SelfHitTestInvisible);
		SetCanTick(true);
	}

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
		ANightSchoolCharacter* Player = Character.Get();
		ANightSchoolDirector* Director = Player ? ANightSchoolDirector::Find(Player->GetWorld()) : nullptr;
		if (!Player || !Director || !StatsText.IsValid())
		{
			return;
		}

		const bool bEnding = Director->IsEnding();
		if (EndingBorder.IsValid())
		{
			EndingBorder->SetVisibility(bEnding ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
		}
		if (bEnding)
		{
			if (EndingTitle.IsValid())
			{
				EndingTitle->SetText(FText::FromString(Director->GetEndingTitle()));
			}
			if (EndingBody.IsValid())
			{
				EndingBody->SetText(FText::FromString(Director->GetEndingBody()));
			}
			if (Crosshair.IsValid())
			{
				Crosshair->SetText(FText::GetEmpty());
			}
			if (PromptText.IsValid())
			{
				PromptText->SetText(FText::GetEmpty());
			}
			return;
		}

		const float Mind = 1.f - FMath::Clamp(Director->GetFear() / 100.f, 0.f, 1.f);
		const float Cell = Player->IsFlashlightOn() ? Player->GetBattery() / 100.f : 0.f;
		StatsText->SetText(FText::FromString(FString::Printf(
			TEXT("心神  %s\n手电  %s%s\n记得  %s\n随身  %s"),
			*Bar(Mind),
			*Bar(Cell),
			Player->IsFlashlightOn() ? TEXT("  亮着") : TEXT("  关着"),
			*Director->GetNamesLine(),
			*Director->GetItemsLine())));
		ObjectiveText->SetText(FText::FromString(Director->GetObjective()));
		StatusText->SetText(FText::FromString(Director->GetStatusLine()));

		FText Prompt;
		if (!Player->IsReading())
		{
			if (ANightSchoolProp* Prop = Cast<ANightSchoolProp>(Player->GetFocusedActor()))
			{
				Prompt = Prop->GetPrompt();
			}
		}
		if (PromptText.IsValid())
		{
			PromptText->SetText(Prompt.IsEmpty() ? FText::GetEmpty() : FText::FromString(FString::Printf(TEXT("按 E    %s"), *Prompt.ToString())));
		}
		if (Crosshair.IsValid())
		{
			Crosshair->SetColorAndOpacity(Prompt.IsEmpty() ? FLinearColor(0.7f, 0.66f, 0.58f, 0.45f) : FLinearColor(0.95f, 0.55f, 0.4f, 0.95f));
		}

		const bool bNote = Director->IsNoteOpen();
		if (NoteBorder.IsValid())
		{
			NoteBorder->SetVisibility(bNote ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
		}
		if (bNote)
		{
			NoteTitle->SetText(FText::FromString(Director->GetNoteTitle()));
			NoteBody->SetText(FText::FromString(Director->GetNoteBody()));
		}
	}

private:
	static FString Bar(float Alpha, int32 Segments = 10)
	{
		const int32 Filled = FMath::Clamp(FMath::RoundToInt(Alpha * Segments), 0, Segments);
		FString Out;
		for (int32 Index = 0; Index < Segments; ++Index)
		{
			Out += (Index < Filled) ? TEXT("■") : TEXT("□");
		}
		return Out;
	}

	TWeakObjectPtr<ANightSchoolCharacter> Character;
	TSharedPtr<STextBlock> TitleText;
	TSharedPtr<STextBlock> StatsText;
	TSharedPtr<STextBlock> ObjectiveText;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> PromptText;
	TSharedPtr<SBorder> NoteBorder;
	TSharedPtr<STextBlock> NoteTitle;
	TSharedPtr<STextBlock> NoteBody;
	TSharedPtr<SBorder> EndingBorder;
	TSharedPtr<STextBlock> EndingTitle;
	TSharedPtr<STextBlock> EndingBody;
	TSharedPtr<STextBlock> Crosshair;
};

TSharedRef<SWidget> MakeNightSchoolHud(ANightSchoolCharacter* Character)
{
	return SNew(SNightSchoolHud).Character(Character);
}
