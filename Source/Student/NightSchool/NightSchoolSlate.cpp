#include "NightSchool/NightSchoolSlate.h"

#include "GameFramework/PlayerController.h"
#include "Input/Reply.h"
#include "Brushes/SlateColorBrush.h"
#include "NightSchool/NightSchoolArt.h"
#include "NightSchool/NightSchoolCharacter.h"
#include "NightSchool/NightSchoolDirector.h"
#include "NightSchool/NightSchoolLayout.h"
#include "NightSchool/NightSchoolProp.h"
#include "NightSchool/NightSchoolStory.h"
#include "Rendering/DrawElementTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FLinearColor Tone(const TCHAR* Hex)
	{
		return FLinearColor::FromSRGBColor(FColor::FromHex(Hex));
	}

	const FLinearColor Ink = Tone(TEXT("F0EEE4"));
	const FLinearColor Muted = Tone(TEXT("B0BEC1"));
	const FLinearColor Ochre = Tone(TEXT("F0BB73"));
	const FLinearColor Sage = Tone(TEXT("A3D3BE"));
	const FLinearColor Rust = Tone(TEXT("F0907E"));
	const FLinearColor Frame = Tone(TEXT("3D4C54"));
	const FLinearColor Surface = Tone(TEXT("141D23"));
	const FLinearColor Panel = Tone(TEXT("202C33"));
	const FLinearColor Selected = Tone(TEXT("344650"));
	const FLinearColor Glass(0.010f, 0.017f, 0.021f, 0.9f);

	const FSlateBrush* Solid()
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	TSharedRef<STextBlock> Text(float Size, const FLinearColor& Color, const FString& Copy = FString(), bool bWrap = false)
	{
		return SNew(STextBlock)
			.Font(NightSchoolArt::FontInfo(Size))
			.ColorAndOpacity(Color)
			.Text(FText::FromString(Copy))
			.AutoWrapText(bWrap)
			.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);
	}

	TSharedRef<STextBlock> BindText(TSharedPtr<STextBlock>& Output, float Size, const FLinearColor& Color,
		const FString& Copy = FString(), bool bWrap = false)
	{
		TSharedRef<STextBlock> Label = Text(Size, Color, Copy, bWrap);
		Output = Label;
		return Label;
	}

	void Update(const TSharedPtr<STextBlock>& Label, const FString& Copy)
	{
		if (Label->GetText().ToString() != Copy)
		{
			Label->SetText(FText::FromString(Copy));
		}
	}

	EVisibility Show(bool bVisible)
	{
		return bVisible ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
	}

	const FProgressBarStyle& MeterStyle()
	{
		static const FProgressBarStyle Style = FProgressBarStyle()
			.SetBackgroundImage(FSlateColorBrush(Frame))
			.SetFillImage(FSlateColorBrush(FLinearColor::White));
		return Style;
	}

	TSharedRef<SBorder> Plate(const FLinearColor& Color, FMargin Padding, const TSharedRef<SWidget>& Child)
	{
		return SNew(SBorder).BorderImage(Solid()).BorderBackgroundColor(Color).Padding(Padding)[Child];
	}

	TSharedRef<SBox> Rule()
	{
		return SNew(SBox).HeightOverride(1.f)[Plate(Frame, FMargin(0.f), SNullWidget::NullWidget)];
	}

	FString ChapterNumber(int32 Chapter)
	{
		static const TCHAR* Labels[] = {TEXT("一"), TEXT("二"), TEXT("三"), TEXT("四"), TEXT("五"), TEXT("六")};
		return FString::Printf(TEXT("第%s章"), Labels[FMath::Clamp(Chapter - 1, 0, 5)]);
	}

	FVector PlayerFeet(const ANightSchoolCharacter& Player)
	{
		return Player.GetActorLocation() - FVector(0.f, 0.f, Player.GetSimpleCollisionHalfHeight());
	}

	TOptional<FVector> TravelTarget(const ANightSchoolCharacter& Player, const FNightSchoolJournalStep* Step)
	{
		if (!Step || !Step->Destination.IsSet())
		{
			return {};
		}
		const TOptional<FVector> Route = NightSchool::RouteWaypoint(PlayerFeet(Player), Step->Destination.GetValue());
		return Route.IsSet() ? Route : Step->Destination;
	}

	class SNightSchoolWaypoint : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SNightSchoolWaypoint) {}
		SLATE_END_ARGS()

		void Construct(const FArguments&) {}

		void UpdateTarget(const ANightSchoolCharacter& Player, const FNightSchoolJournalStep* Step)
		{
			const APlayerController* Controller = Cast<APlayerController>(Player.GetController());
			int32 Width = 0;
			int32 Height = 0;
			if (Controller)
			{
				Controller->GetViewportSize(Width, Height);
			}
			const TOptional<FVector> Destination = TravelTarget(Player, Step);
			bHasTarget = Destination.IsSet() && Width > 0 && Height > 0;
			if (bHasTarget)
			{
				const FVector Target = Destination.GetValue();
				const bool bOnRoute = FVector::DistSquared(Target, Step->Destination.GetValue()) > 1.0;
				const FVector Elevated = Target + FVector(0.0, 0.0, bOnRoute ? 28.0 : 49.0);
				Distance = FMath::RoundToInt(FVector::Dist(Player.GetActorLocation(), Target) / 100.0);
				FVector2D Projected;
				bInFront = Controller->ProjectWorldLocationToScreen(Elevated, Projected);
				if (bInFront)
				{
					ScreenPoint = FVector2f(Projected.X / Width, Projected.Y / Height);
				}
				else
				{
					FVector ViewOrigin;
					FRotator ViewRotation;
					Controller->GetPlayerViewPoint(ViewOrigin, ViewRotation);
					const FRotationMatrix ViewAxes(ViewRotation);
					const FVector Offset = Elevated - ViewOrigin;
					const double Lateral = FVector::DotProduct(Offset, ViewAxes.GetUnitAxis(EAxis::Y));
					const double Depth = FVector::DotProduct(Offset, ViewAxes.GetUnitAxis(EAxis::X));
					RearDirection = FVector2f(Lateral, FMath::Max(1.0, FMath::Abs(Depth) * .35)).GetSafeNormal();
				}
			}
			Invalidate(EInvalidateWidgetReason::Paint);
		}

		FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }

		int32 OnPaint(const FPaintArgs&, const FGeometry& Geo, const FSlateRect&, FSlateWindowElementList& Out,
			int32 Layer, const FWidgetStyle&, bool) const override
		{
			if (!bHasTarget)
			{
				return Layer;
			}
			const FVector2f Size(Geo.GetLocalSize());
			if (Size.X < 180.f || Size.Y < 240.f)
			{
				return Layer;
			}

			const FVector2f Center = Size * .5f;
			const FVector2f Min(43.f, FMath::Min(115.f, Size.Y * .28f));
			const FVector2f Max(Size.X - 43.f, Size.Y - FMath::Min(98.f, Size.Y * .23f));
			FVector2f Position(ScreenPoint.X * Size.X, ScreenPoint.Y * Size.Y);
			const bool bAtEdge = !bInFront || Position.X < Min.X || Position.X > Max.X
				|| Position.Y < Min.Y || Position.Y > Max.Y;
			FVector2f Direction = FVector2f::ZeroVector;
			if (bAtEdge)
			{
				Direction = bInFront ? (Position - Center).GetSafeNormal() : RearDirection;
				const FVector2f Limits(Direction.X < 0.f ? Center.X - Min.X : Max.X - Center.X,
					Direction.Y < 0.f ? Center.Y - Min.Y : Max.Y - Center.Y);
				const float EdgeDistance = 1.f / FMath::Max(FMath::Abs(Direction.X) / Limits.X,
					FMath::Abs(Direction.Y) / Limits.Y);
				Position = Center + Direction * EdgeDistance;
			}

			const FPaintGeometry Paint = Geo.ToPaintGeometry();
			auto Outline = [&](const TArray<FVector2f>& Points)
			{
				FSlateDrawElement::MakeLines(Out, Layer, Paint, Points, ESlateDrawEffect::None, Glass, true, 5.f);
				FSlateDrawElement::MakeLines(Out, Layer + 1, Paint, Points, ESlateDrawEffect::None, Ochre, true, 2.f);
			};
			if (bAtEdge)
			{
				const FVector2f Side(-Direction.Y, Direction.X);
				Outline({Position - Direction * 4.f + Side * 7.f, Position + Direction * 5.f,
					Position - Direction * 4.f - Side * 7.f});
			}
			else
			{
				const FVector2f Up(0.f, -8.f);
				const FVector2f Right(8.f, 0.f);
				Outline({Position + Up, Position + Right, Position - Up, Position - Right, Position + Up});
			}

			const FVector2f LabelOrigin = Position + FVector2f(-21.f, 12.f);
			FSlateDrawElement::MakeBox(Out, Layer + 1,
				Geo.ToPaintGeometry(FVector2f(42.f, 18.f), FSlateLayoutTransform(LabelOrigin)),
				Solid(), ESlateDrawEffect::None, Glass);
			const FString Label = FString::Printf(TEXT("%d m"), Distance);
			FSlateDrawElement::MakeText(Out, Layer + 2,
				Geo.ToPaintGeometry(FVector2f(42.f, 18.f),
					FSlateLayoutTransform(LabelOrigin + FVector2f(21.f - Label.Len() * 3.3f, 1.f))),
				Label, NightSchoolArt::FontInfo(11.f), ESlateDrawEffect::None, Ink);
			return Layer + 2;
		}

	private:
		FVector2f ScreenPoint = FVector2f::ZeroVector;
		FVector2f RearDirection = FVector2f(0.f, 1.f);
		int32 Distance = 0;
		bool bHasTarget = false;
		bool bInFront = false;
	};

	class SNightSchoolFloorSketch : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SNightSchoolFloorSketch) {}
		SLATE_END_ARGS()

		void Construct(const FArguments&) {}

		void SetContents(const TArray<FNightSchoolJournalRoom>& Known, const FVector& Here, const TOptional<FVector>& Goal)
		{
			const int32 Floor = NightSchool::FloorAt(Here);
			Rooms.Reset();
			for (const FNightSchoolJournalRoom& Room : Known)
			{
				if (NightSchool::FloorAt(Room.Bounds.Min) == Floor)
				{
					Rooms.Add(Room);
				}
			}
			Player = Here;
			Destination = Goal;
			Invalidate(EInvalidateWidgetReason::Paint);
		}

		FVector2D ComputeDesiredSize(float) const override { return FVector2D(218.f, 202.f); }

		int32 OnPaint(const FPaintArgs&, const FGeometry& Geo, const FSlateRect&, FSlateWindowElementList& Out,
			int32 Layer, const FWidgetStyle&, bool) const override
		{
			FBox Bounds(ForceInit);
			Bounds += Player;
			for (const FNightSchoolJournalRoom& Room : Rooms)
			{
				Bounds += Room.Bounds;
			}
			if (Destination.IsSet())
			{
				Bounds += Destination.GetValue();
			}
			const FVector Center = Bounds.GetCenter();
			const FVector Span = Bounds.GetSize();
			const FVector2f Size(Geo.GetLocalSize());
			const float Scale = FMath::Min((Size.X - 39.f) / FMath::Max(780.f, static_cast<float>(Span.X)),
				(Size.Y - 46.f) / FMath::Max(700.f, static_cast<float>(Span.Y)));
			auto Point = [&](const FVector& P)
			{
				return Size * .5f + FVector2f((P.X - Center.X) * Scale, (Center.Y - P.Y) * Scale);
			};
			auto Rect = [&](int32 Offset, const FVector2f& At, const FVector2f& Dimensions, FLinearColor Color)
			{
				FSlateDrawElement::MakeBox(Out, Layer + Offset, Geo.ToPaintGeometry(Dimensions, FSlateLayoutTransform(At)),
					Solid(), ESlateDrawEffect::None, Color);
			};
			for (const FNightSchoolJournalRoom& Room : Rooms)
			{
				const FVector2f First = Point(FVector(Room.Bounds.Min.X, Room.Bounds.Max.Y, 0.f));
				const FVector2f Last = Point(FVector(Room.Bounds.Max.X, Room.Bounds.Min.Y, 0.f));
				const FVector2f Area = Last - First;
				Rect(0, First, Area, Frame);
				if (Area.X > 3.f && Area.Y > 3.f)
				{
					Rect(1, First + FVector2f(1.f), Area - FVector2f(2.f), Selected);
				}
				if (Area.X > Room.Name.Len() * 11.f + 6.f && Area.Y > 19.f)
				{
					FSlateDrawElement::MakeText(Out, Layer + 2,
						Geo.ToPaintGeometry(Area, FSlateLayoutTransform(First + FVector2f(4.f, 3.f))),
						Room.Name, NightSchoolArt::FontInfo(11.f), ESlateDrawEffect::None, Ink);
				}
			}
			if (Destination.IsSet())
			{
				Rect(3, Point(Destination.GetValue()) - FVector2f(4.f), FVector2f(8.f), Ochre);
			}
			Rect(4, Point(Player) + FVector2f(-3.f, 3.f), FVector2f(6.f), Sage);
			FSlateDrawElement::MakeText(Out, Layer + 4,
				Geo.ToPaintGeometry(FVector2f(22.f), FSlateLayoutTransform(FVector2f(Size.X - 23.f, 7.f))),
				TEXT("北"), NightSchoolArt::FontInfo(12.f), ESlateDrawEffect::None, Muted);
			return Layer + 4;
		}

	private:
		TArray<FNightSchoolJournalRoom> Rooms;
		FVector Player = FVector::ZeroVector;
		TOptional<FVector> Destination;
	};
}

class SNightSchoolHud : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNightSchoolHud) {}
		SLATE_ARGUMENT(ANightSchoolCharacter*, Character)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args)
	{
		Character = Args._Character;
		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()[SAssignNew(PlayHud, SOverlay)]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(26.f, 26.f, 26.f, 35.f)
			[
				SAssignNew(PreviewLayer, SBorder).Visibility(EVisibility::Collapsed)
				.BorderImage(Solid()).BorderBackgroundColor(Glass).Padding(FMargin(18.f, 9.f))
				[SNew(SBox).MaxDesiredWidth(660.f)[BindText(Preview, 16.f, Ink, FString(), true)]]
			]
			+ SOverlay::Slot()[MakeReader()]
			+ SOverlay::Slot()[MakeJournal()]
			+ SOverlay::Slot()[MakeEnding()]
		];
		MakePlayHud();
		SetVisibility(EVisibility::SelfHitTestInvisible);
		SetCanTick(true);
	}

	bool SupportsKeyboardFocus() const override { return true; }

	FReply OnPreviewKeyDown(const FGeometry&, const FKeyEvent& Event) override
	{
		ANightSchoolCharacter* Player = Character.Get();
		if (!Player)
		{
			return FReply::Unhandled();
		}
		const FKey Key = Event.GetKey();
		if (Player->IsReading() && (Key == EKeys::Tab || Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right))
		{
			if (!Event.IsRepeat())
			{
				if (ANightSchoolDirector* Story = Director.Get())
				{
					Story->CloseNote();
				}
			}
			return FReply::Handled();
		}
		if (!Player->IsJournalOpen())
		{
			return FReply::Unhandled();
		}
		if (Key == EKeys::Escape || Key == EKeys::Tab || Key == EKeys::J
			|| Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_Special_Left)
		{
			if (!Event.IsRepeat())
			{
				Player->SetJournalOpen(false);
			}
			return FReply::Handled();
		}
		if (Key == EKeys::Left || Key == EKeys::Right || Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Gamepad_DPad_Right)
		{
			SwitchTab(Key == EKeys::Right || Key == EKeys::Gamepad_DPad_Right);
			return FReply::Handled();
		}
		if (Key == EKeys::Up || Key == EKeys::Down || Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_DPad_Down)
		{
			const int32 Direction = Key == EKeys::Up || Key == EKeys::Gamepad_DPad_Up ? -1 : 1;
			if (bSideTab)
			{
				MoveSelection(Direction);
			}
			else
			{
				StepScroll->SetScrollOffset(FMath::Clamp(StepScroll->GetScrollOffset() + Direction * 62.f,
					0.f, FMath::Max(0.f, StepScroll->GetScrollOffsetOfEnd())));
			}
			return FReply::Handled();
		}
		if (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom)
		{
			if (!Event.IsRepeat())
			{
				ToggleTracking();
			}
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	void Tick(const FGeometry& Geo, double Now, float Delta) override
	{
		SCompoundWidget::Tick(Geo, Now, Delta);
		ANightSchoolCharacter* Player = Character.Get();
		if (!Player)
		{
			return;
		}
		if (!Director.IsValid())
		{
			Director = ANightSchoolDirector::Find(Player->GetWorld());
		}
		const ANightSchoolDirector* Story = Director.Get();
		if (!Story)
		{
			PlayHud->SetVisibility(EVisibility::Collapsed);
			return;
		}

		const bool bEnding = Story->IsEnding();
		const bool bReading = Story->IsNoteOpen() && !bEnding;
		const bool bPreviewing = Story->IsPreviewing() && !bEnding;
		const bool bJournal = Player->IsJournalOpen() && !bEnding;
		PlayHud->SetVisibility(Show(!bEnding && !bReading && !bPreviewing && !bJournal));
		PreviewLayer->SetVisibility(Show(bPreviewing));
		JournalLayer->SetVisibility(bJournal ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
		NoteLayer->SetVisibility(bReading ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
		EndingLayer->SetVisibility(Show(bEnding));

		if (bEnding)
		{
			Update(EndingTitle, Story->GetEndingTitle());
			Update(EndingBody, Story->GetEndingBody());
			return;
		}
		if (bReading)
		{
			Update(NoteTitle, Story->GetNoteTitle());
			if (NoteBody->GetText().ToString() != Story->GetNoteBody())
			{
				Update(NoteBody, Story->GetNoteBody());
				NoteScroll->ScrollToStart();
			}
		}
		if (bPreviewing)
		{
			Update(Preview, Story->GetPreviewLine());
		}
		if (Now >= RefreshAt || (bJournal && !bJournalWasOpen))
		{
			RefreshAt = Now + 0.18;
			Story->BuildJournal(Entries);
			CurrentChapter = Story->GetChapterIndex();
			TimedObjective = Story->GetTimedObjective();
			UpdateObjectiveToast(Now);
		}
		if (bJournal)
		{
			if (!bJournalWasOpen)
			{
				const FNightSchoolJournalEntry* Tracked = TrackedEntry();
				bSideTab = Tracked && Tracked->Chapter == 0;
				const FNightSchoolJournalEntry* Main = MainEntry();
				Selection = Tracked ? Tracked->Id : Main ? Main->Id : NAME_None;
				Story->GetJournalRooms(KnownRooms);
				Update(Inventory, Story->GetItemsLine());
				RebuildJournal();
			}
			Update(JournalKeys, Player->UsesGamepadHints()
				? (bSideTab ? TEXT("方向键  选择 / 切换     A  追踪     B  返回")
					: TEXT("上下键  滚动     左右键  切换     A  追踪     B  返回"))
				: (bSideTab ? TEXT("上下键  选择     左右键  切换     Enter  追踪     J / Esc  返回")
					: TEXT("上下键  滚动     左右键  切换     Enter  追踪     J / Esc  返回")));
		}
		bJournalWasOpen = bJournal;
		if (!bReading && !bPreviewing && !bJournal)
		{
			UpdatePlayHud(*Player, *Story, Now);
		}
	}

private:
	TSharedRef<SWidget> Choice(const FString& Label, const FLinearColor& Color, const FLinearColor& Back,
		FOnClicked OnClicked, FMargin Padding = FMargin(12.f, 8.f))
	{
		return SNew(SButton)
			.ButtonStyle(FCoreStyle::Get(), "NoBorder")
			.ContentPadding(FMargin(0.f))
			.OnClicked(OnClicked)
			[Plate(Back, Padding, Text(16.f, Color, Label))];
	}

	void MakePlayHud()
	{
		PlayHud->AddSlot()[SAssignNew(Waypoint, SNightSchoolWaypoint).Visibility(EVisibility::HitTestInvisible)];
		PlayHud->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(22.f, 22.f, 0.f, 0.f)
		[
			SAssignNew(ObjectivePane, SBorder).BorderImage(Solid()).BorderBackgroundColor(Glass).Padding(FMargin(12.f, 10.f, 17.f, 10.f))
			[
				SNew(SBox).MinDesiredWidth(230.f).MaxDesiredWidth(315.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[BindText(ObjectiveContext, 13.f, Ochre)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)[BindText(Objective, 21.f, Ink, FString(), true)]
					+ SVerticalBox::Slot().AutoHeight()[BindText(Bearing, 13.f, Muted)]
				]
			]
		];
		PlayHud->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 23.f, 22.f, 0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
			[Plate(Glass, FMargin(13.f, 8.f), Meter(TEXT("心神"), Sage, MindBar, 168.f, 7.f))]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 9.f, 0.f, 0.f)
			[
				SAssignNew(ToastLayer, SBorder).BorderImage(Solid()).BorderBackgroundColor(Glass).Padding(FMargin(13.f, 9.f))
				[SNew(SBox).WidthOverride(285.f)[BindText(Toast, 15.f, Ochre, FString(), true)]]
			]
		];
		PlayHud->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(22.f, 22.f, 22.f, 0.f)
		[
			SAssignNew(TimerLayer, SBorder).BorderImage(Solid()).BorderBackgroundColor(Glass).Padding(FMargin(19.f, 8.f))
			[BindText(Timer, 20.f, Rust)]
		];
		PlayHud->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[BindText(Crosshair, 18.f, Muted, TEXT("·"))]
		];
		PlayHud->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(20.f, 20.f, 20.f, 48.f)
		[
			SAssignNew(InteractionPane, SBorder).BorderImage(Solid()).BorderBackgroundColor(Glass).Padding(FMargin(16.f, 9.f))
			[BindText(Interaction, 19.f, Ink)]
		];
		PlayHud->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(22.f)
		[
			Plate(Glass, FMargin(13.f, 8.f), Meter(TEXT("手电"), Ochre, PowerBar, 74.f, 5.f))
		];
		PlayHud->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(22.f)
		[
			Plate(Glass, FMargin(12.f, 7.f), BindText(JournalHint, 14.f, Ink, TEXT("J   任务日志")))
		];
	}

	TSharedRef<SWidget> Meter(const TCHAR* Label, FLinearColor Color, TSharedPtr<SProgressBar>& Bar, float Width, float Height)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[Text(13.f, Muted, Label)]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
			[
				SNew(SBox).WidthOverride(Width).HeightOverride(Height)
				[SAssignNew(Bar, SProgressBar).Style(&MeterStyle()).Percent(1.f).FillColorAndOpacity(Color)]
			];
	}

	TSharedRef<SWidget> MakeReader()
	{
		return SAssignNew(NoteLayer, SOverlay).Visibility(EVisibility::Collapsed)
			+ SOverlay::Slot()[Plate(FLinearColor(0.f, 0.f, 0.f, .4f), FMargin(0.f), SNullWidget::NullWidget)]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(26.f)
			[
				SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
				[
					Plate(Surface, FMargin(27.f, 24.f),
						SNew(SBox).WidthOverride(635.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[BindText(NoteTitle, 28.f, Ochre)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f)[Rule()]
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(SBox).MaxDesiredHeight(338.f)
								[
									SAssignNew(NoteScroll, SScrollBox)
									+ SScrollBox::Slot().Padding(0.f, 3.f, 16.f, 5.f)[BindText(NoteBody, 19.f, Ink, FString(), true)]
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)[Text(13.f, Muted, TEXT("Tab / Esc   合上"))]
						])
				]
			];
	}

	TSharedRef<SWidget> MakeEnding()
	{
		return SAssignNew(EndingLayer, SOverlay).Visibility(EVisibility::Collapsed)
			+ SOverlay::Slot()[Plate(FLinearColor(0.006f, 0.008f, 0.009f, .98f), FMargin(0.f), SNullWidget::NullWidget)]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(35.f)
			[
				SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
				[
					SNew(SBox).WidthOverride(735.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)[BindText(EndingTitle, 34.f, Ochre)]
						+ SVerticalBox::Slot().AutoHeight()[SNew(SBox).MaxDesiredHeight(450.f)[
							SNew(SScrollBox) + SScrollBox::Slot()[BindText(EndingBody, 19.f, Ink, FString(), true)]]]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 23.f, 0.f, 0.f)[Text(14.f, Muted, TEXT("R   重走走廊"))]
					]
				]
			];
	}

	TSharedRef<SWidget> MakeJournal()
	{
		return SAssignNew(JournalLayer, SOverlay).Visibility(EVisibility::Collapsed)
			+ SOverlay::Slot()[Plate(FLinearColor(0.003f, 0.007f, 0.011f, .8f), FMargin(0.f), SNullWidget::NullWidget)]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(22.f)
			[
				SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
				[
					SNew(SBox).WidthOverride(1136.f).HeightOverride(638.f)
					[
						Plate(Frame, FMargin(1.f),
							Plate(Surface, FMargin(22.f, 18.f),
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()[MakeJournalHeader()]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 13.f, 0.f, 14.f)[Rule()]
								+ SVerticalBox::Slot().FillHeight(1.f)[MakeJournalColumns()]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 13.f, 0.f, 8.f)[Rule()]
								+ SVerticalBox::Slot().AutoHeight()[BindText(JournalKeys, 13.f, Muted)]))
					]
				]
			];
	}

	TSharedRef<SWidget> MakeJournalHeader()
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[Text(32.f, Ink, TEXT("任务日志"))]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f)
			[SAssignNew(MainTab, SBox)]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 17.f, 0.f)
			[SAssignNew(SideTab, SBox)]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				Choice(TEXT("返回"), Muted, Panel, FOnClicked::CreateLambda([this]
				{
					if (ANightSchoolCharacter* Player = Character.Get())
					{
						Player->SetJournalOpen(false);
					}
					return FReply::Handled();
				}))
			];
	}

	TSharedRef<SWidget> MakeJournalColumns()
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(SideList, SBox).WidthOverride(244.f).Padding(FMargin(0.f, 0.f, 14.f, 0.f))
				[
					Plate(Panel, FMargin(10.f),
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(4.f, 3.f, 0.f, 10.f)[BindText(ListTitle, 14.f, Muted)]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SAssignNew(EntryScroll, SScrollBox)
							+ SScrollBox::Slot().Padding(0.f, 0.f, 5.f, 0.f)[SAssignNew(EntryRows, SVerticalBox)]
						])
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
			[
				Plate(Panel, FMargin(17.f, 15.f),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[BindText(EntryState, 13.f, Ochre)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)[BindText(EntryTitle, 27.f, Ink)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 10.f)[BindText(EntrySummary, 16.f, Muted, FString(), true)]
					+ SVerticalBox::Slot().AutoHeight()[Rule()]
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 10.f, 0.f, 8.f)
					[
						SAssignNew(StepScroll, SScrollBox)
						+ SScrollBox::Slot().Padding(0.f, 0.f, 10.f, 0.f)[SAssignNew(StepRows, SVerticalBox)]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)[SAssignNew(TrackButton, SBox)])
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox).WidthOverride(232.f)
				[
					Plate(Panel, FMargin(11.f, 14.f),
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()[BindText(MapTitle, 16.f, Ink)]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 9.f)
						[
							Plate(Surface, FMargin(0.f), SAssignNew(FloorSketch, SNightSchoolFloorSketch))
						]
						+ SVerticalBox::Slot().AutoHeight()[Text(12.f, Muted, TEXT("绿色  你的位置    金色  目标"), true)]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 13.f)[Rule()]
						+ SVerticalBox::Slot().AutoHeight()[Text(16.f, Ink, TEXT("随身物品"))]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 7.f)[BindText(Inventory, 14.f, Muted, FString(), true)]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 15.f, 0.f, 0.f)
						[Text(13.f, Muted, TEXT("靠近红印台，可用交互键留存进度。"), true)])
				]
			];
	}

	const FNightSchoolJournalEntry* Find(FName Id) const
	{
		return Entries.FindByPredicate([Id](const FNightSchoolJournalEntry& Entry) { return Entry.Id == Id; });
	}

	const FNightSchoolJournalEntry* MainEntry() const
	{
		return Entries.FindByPredicate([this](const FNightSchoolJournalEntry& Entry) { return Entry.Chapter == CurrentChapter; });
	}

	const FNightSchoolJournalEntry* TrackedEntry() const
	{
		if (!bTracking)
		{
			return nullptr;
		}
		const FNightSchoolJournalEntry* Side = Find(TrackedSide);
		return Side && Side->Next() ? Side : MainEntry();
	}

	TArray<FName> SideEntries() const
	{
		TArray<FName> Ids;
		for (const FNightSchoolJournalEntry& Entry : Entries)
		{
			if (Entry.Chapter == 0)
			{
				Ids.Add(Entry.Id);
			}
		}
		return Ids;
	}

	void SwitchTab(bool bSide)
	{
		if (bSideTab == bSide)
		{
			return;
		}
		bSideTab = bSide;
		const TArray<FName> Rows = bSide ? SideEntries() : TArray<FName>();
		const FNightSchoolJournalEntry* Main = MainEntry();
		Selection = !bSide && Main ? Main->Id : (Rows.IsEmpty() ? NAME_None : Rows[0]);
		RebuildJournal();
	}

	void MoveSelection(int32 Direction)
	{
		const TArray<FName> Rows = SideEntries();
		if (Rows.Num() < 2)
		{
			return;
		}
		const int32 At = FMath::Max(Rows.IndexOfByKey(Selection), 0);
		Selection = Rows[FMath::Clamp(At + Direction, 0, Rows.Num() - 1)];
		RebuildJournal();
	}

	void ToggleTracking()
	{
		const FNightSchoolJournalEntry* Entry = Find(Selection);
		if (!Entry || !Entry->Next())
		{
			return;
		}
		if (TrackedEntry() == Entry)
		{
			bTracking = false;
			TrackedSide = NAME_None;
		}
		else
		{
			bTracking = true;
			TrackedSide = Entry->Chapter == 0 ? Entry->Id : NAME_None;
		}
		RebuildJournal();
	}

	void RebuildJournal()
	{
		MainTab->SetContent(Choice(TEXT("主线"), bSideTab ? Muted : Ochre, bSideTab ? Panel : Selected,
			FOnClicked::CreateLambda([this] { SwitchTab(false); return FReply::Handled(); })));
		SideTab->SetContent(Choice(TEXT("校园支线"), bSideTab ? Ochre : Muted, bSideTab ? Selected : Panel,
			FOnClicked::CreateLambda([this] { SwitchTab(true); return FReply::Handled(); })));
		EntryRows->ClearChildren();
		StepRows->ClearChildren();
		StepScroll->ScrollToStart();
		const FNightSchoolJournalEntry* Tracked = TrackedEntry();
		const TArray<FName> Ids = bSideTab ? SideEntries() : TArray<FName>();
		SideList->SetVisibility(bSideTab ? EVisibility::Visible : EVisibility::Collapsed);
		Update(ListTitle, FString::Printf(TEXT("已发现  %d"), Ids.Num()));
		for (FName Id : Ids)
		{
			const FNightSchoolJournalEntry& Entry = *Find(Id);
			const bool bSelected = Id == Selection;
			const TCHAR* RowState = Entry.bComplete ? TEXT("已完成")
				: Tracked == &Entry ? TEXT("追踪中") : Entry.Next() ? TEXT("进行中") : TEXT("待发现");
			const FLinearColor Back = bSelected ? Selected : Surface;
			const FLinearColor Accent = bSelected || Tracked == &Entry ? Ochre : Frame;
			TSharedRef<SButton> Row = SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.ContentPadding(FMargin(0.f))
				.OnClicked_Lambda([this, Id] { Selection = Id; RebuildJournal(); return FReply::Handled(); })
				[
					Plate(Accent, FMargin(2.f, 0.f, 0.f, 0.f),
						Plate(Back, FMargin(10.f, 7.f),
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[Text(17.f, Ink, Entry.Title)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)[Text(12.f, Tracked == &Entry ? Ochre : Muted, RowState)]))
				];
			EntryRows->AddSlot().AutoHeight().Padding(0.f, 0.f, 1.f, 6.f)[Row];
			if (bSelected)
			{
				EntryScroll->ScrollDescendantIntoView(Row, false);
			}
		}

		const FNightSchoolJournalEntry* Pick = Find(Selection);
		const FNightSchoolJournalStep* Active = Pick ? Pick->Next() : nullptr;
		TrackButton->SetContent(SNullWidget::NullWidget);
		if (const ANightSchoolCharacter* Player = Character.Get())
		{
			const FVector Feet = PlayerFeet(*Player);
			Update(MapTitle, FString(NightSchool::FloorName(NightSchool::FloorAt(Feet))) + TEXT(" · 已探索区域"));
			FloorSketch->SetContents(KnownRooms, Feet, TravelTarget(*Player, Active));
		}
		if (!Pick)
		{
			Update(EntryState, TEXT("校园支线"));
			Update(EntryTitle, TEXT("还没有发现支线"));
			Update(EntrySummary, TEXT("继续调查宿舍和校园，发现的支线会记在这里。"));
			return;
		}
		Update(EntryState, Pick->Chapter > 0 ? ChapterNumber(Pick->Chapter) : TEXT("校园支线"));
		Update(EntryTitle, Pick->Title);
		Update(EntrySummary, Pick->bLocked ? FString(TEXT("完成上一章后解锁。")) : Pick->Summary);
		if (TimedObjective.IsSet() && Pick == Tracked && !Pick->bLocked)
		{
			const FNightSchoolJournalStep& Puzzle = TimedObjective.GetValue();
			StepRows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 9.f)
			[
				Plate(Surface, FMargin(10.f, 7.f),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[Text(16.f, Rust, FString(TEXT("进行中的解谜  ·  ")) + Puzzle.Title)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[Text(14.f, Muted, Puzzle.Hint, true)])
			];
		}
		for (const FNightSchoolJournalStep& Step : Pick->Steps)
		{
			const bool bDone = Step.bDone || Pick->bComplete;
			const bool bNext = Active == &Step;
			if (bSideTab && !bNext && !bDone)
			{
				continue;
			}
			const TCHAR* StepState = bNext ? TEXT("当前  ") : bDone ? TEXT("已完成  ") : TEXT("待完成  ");
			StepRows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				Plate(bNext ? Selected : Surface, FMargin(10.f, 7.f),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						Text(bNext ? 20.f : 15.f, bNext ? Ink : Muted,
							FString(StepState) + Step.Title)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SNew(STextBlock).Font(NightSchoolArt::FontInfo(15.f)).ColorAndOpacity(Ochre)
						.Text(FText::FromString(Step.Place + TEXT("  ·  ") + Step.Hint))
						.AutoWrapText(true).WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
						.Visibility(Show(bNext))
					])
			];
		}
		if (Active)
		{
			TrackButton->SetContent(Choice(Tracked == Pick ? TEXT("取消追踪") : TEXT("追踪任务"),
				Tracked == Pick ? Muted : Surface, Tracked == Pick ? Selected : Ochre,
				FOnClicked::CreateLambda([this] { ToggleTracking(); return FReply::Handled(); })));
		}
		else if (!Pick->bLocked && !Pick->bComplete)
		{
			StepRows->AddSlot().AutoHeight()[Text(15.f, Muted, TEXT("继续主线后，会出现更多可调查的地点。"))];
		}
	}

	void UpdateObjectiveToast(double Now)
	{
		const FNightSchoolJournalEntry* Main = MainEntry();
		const FNightSchoolJournalStep* Step = Main ? Main->Next() : nullptr;
		if (!Step)
		{
			return;
		}
		if (!LastObjective.IsNone() && LastObjective != Step->Id)
		{
			ProgressToast = LastChapter != CurrentChapter
				? FString(NightSchoolStory::ChapterBanner(CurrentChapter))
				: FString(TEXT("目标更新  ·  ")) + Step->Title;
			ToastThrough = Now + 4.5;
		}
		LastObjective = Step->Id;
		LastChapter = CurrentChapter;
	}

	void UpdatePlayHud(const ANightSchoolCharacter& Player, const ANightSchoolDirector& Story, double Now)
	{
		const int32 Seconds = Story.GetPuzzleSecondsLeft();
		if (Seconds >= 0 && !TimedObjective.IsSet())
		{
			TimedObjective = Story.GetTimedObjective();
		}
		else if (Seconds < 0)
		{
			TimedObjective.Reset();
		}
		const FNightSchoolJournalEntry* Tracked = TrackedEntry();
		const FNightSchoolJournalStep* TrackedStep = Tracked ? Tracked->Next() : nullptr;
		const FNightSchoolJournalStep* Step = TimedObjective.IsSet() ? &TimedObjective.GetValue() : TrackedStep;
		Waypoint->UpdateTarget(Player, TrackedStep);
		ObjectivePane->SetVisibility(Show(Step != nullptr));
		if (Step)
		{
			Update(ObjectiveContext, TimedObjective.IsSet() ? FString(TEXT("限时解谜"))
				: Tracked->Chapter ? ChapterNumber(Tracked->Chapter) + TEXT("  /  ") + Tracked->Title
				: FString(TEXT("支线  /  ")) + Tracked->Title);
			Update(Objective, Step->Title);
			Update(Bearing, RouteLabel(Player, *Step));
		}

		TimerLayer->SetVisibility(Show(Seconds >= 0));
		if (Seconds >= 0)
		{
			Update(Timer, FString::Printf(TEXT("解谜限时   %02d:%02d"), Seconds / 60, Seconds % 60));
		}

		const FString& Status = Story.GetStatusLine();
		const FString DisplayToast = !Status.IsEmpty() ? Status : (Now < ToastThrough ? ProgressToast : FString());
		ToastLayer->SetVisibility(Show(!DisplayToast.IsEmpty()));
		Update(Toast, DisplayToast);

		const ANightSchoolProp* Prop = Cast<ANightSchoolProp>(Player.GetFocusedActor());
		const FText Prompt = Prop ? Prop->GetPrompt() : FText::GetEmpty();
		InteractionPane->SetVisibility(Show(!Prompt.IsEmpty()));
		const bool bPad = Player.UsesGamepadHints();
		if (!Prompt.IsEmpty())
		{
			Update(Interaction, (bPad ? FString(TEXT("A    ")) : FString(TEXT("G    "))) + Prompt.ToString());
		}
		Update(JournalHint, bPad ? TEXT("View   任务日志") : TEXT("J   任务日志"));
		const float Lean = Player.GetLean();
		Update(Crosshair, Lean < -.35f ? TEXT("‹") : Lean > .35f ? TEXT("›") : TEXT("·"));
		Crosshair->SetColorAndOpacity(Prompt.IsEmpty() ? Muted.CopyWithNewOpacity(.5f) : Ochre);
		MindBar->SetPercent(FMath::Clamp(1.f - Story.GetFear() / 100.f, 0.f, 1.f));
		PowerBar->SetPercent(Player.GetBattery() / 100.f);
		PowerBar->SetFillColorAndOpacity(Player.IsFlashlightOn() ? Ochre : Muted.CopyWithNewOpacity(.6f));
	}

	static FString RouteLabel(const ANightSchoolCharacter& Player, const FNightSchoolJournalStep& Step)
	{
		if (!Step.Destination.IsSet())
		{
			return Step.Place;
		}
		const FVector Destination = Step.Destination.GetValue();
		const FVector Feet = PlayerFeet(Player);
		const int32 TargetFloor = NightSchool::FloorAt(Destination);
		const TOptional<FVector> Route = NightSchool::RouteWaypoint(Feet, Destination);
		const FVector Delta = (Route.IsSet() ? Route.GetValue() : Destination) - Player.GetActorLocation();
		const FString Place = TargetFloor != NightSchool::FloorAt(Feet)
			? FString::Printf(TEXT("去%s · 经楼梯"), NightSchool::FloorName(TargetFloor)) : Step.Place;
		const int32 Meters = FMath::RoundToInt(Delta.Size() / 100.f);
		const AController* Controller = Player.GetController();
		const float Angle = FMath::FindDeltaAngleDegrees(Controller ? Controller->GetControlRotation().Yaw : Player.GetActorRotation().Yaw,
			Delta.Rotation().Yaw);
		const TCHAR* Direction = Meters <= 2 ? TEXT("就在附近")
			: FMath::Abs(Angle) < 30.f ? TEXT("前方")
			: FMath::Abs(Angle) > 150.f ? TEXT("身后")
			: Angle < 0.f ? TEXT("左侧") : TEXT("右侧");
		return Meters <= 2 ? Place + TEXT("   ·   ") + Direction
			: FString::Printf(TEXT("%s   ·   %s %d 米"), *Place, Direction, Meters);
	}

	TWeakObjectPtr<ANightSchoolCharacter> Character;
	TWeakObjectPtr<ANightSchoolDirector> Director;
	TArray<FNightSchoolJournalEntry> Entries;
	TArray<FNightSchoolJournalRoom> KnownRooms;
	TOptional<FNightSchoolJournalStep> TimedObjective;
	FName Selection;
	FName TrackedSide;
	FName LastObjective;
	FString ProgressToast;
	double RefreshAt = 0.0;
	double ToastThrough = 0.0;
	int32 CurrentChapter = 1;
	int32 LastChapter = 1;
	bool bSideTab = false;
	bool bTracking = true;
	bool bJournalWasOpen = false;

	TSharedPtr<SOverlay> PlayHud, NoteLayer, JournalLayer, EndingLayer;
	TSharedPtr<SBorder> ObjectivePane, ToastLayer, TimerLayer, InteractionPane, PreviewLayer;
	TSharedPtr<SBox> TrackButton, MainTab, SideTab, SideList;
	TSharedPtr<SVerticalBox> EntryRows, StepRows;
	TSharedPtr<SScrollBox> EntryScroll, StepScroll, NoteScroll;
	TSharedPtr<SProgressBar> MindBar, PowerBar;
	TSharedPtr<SNightSchoolFloorSketch> FloorSketch;
	TSharedPtr<SNightSchoolWaypoint> Waypoint;
	TSharedPtr<STextBlock> ObjectiveContext, Objective, Bearing, Toast, Timer, Crosshair, Interaction, JournalHint, Preview;
	TSharedPtr<STextBlock> ListTitle, EntryState, EntryTitle, EntrySummary, JournalKeys, Inventory, MapTitle;
	TSharedPtr<STextBlock> NoteTitle, NoteBody, EndingTitle, EndingBody;
};

TSharedRef<SWidget> MakeNightSchoolHud(ANightSchoolCharacter* Character)
{
	return SNew(SNightSchoolHud).Character(Character);
}
