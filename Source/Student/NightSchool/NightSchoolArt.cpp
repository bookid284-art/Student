#include "NightSchool/NightSchoolArt.h"

#include "Engine/Font.h"
#include "Engine/FontFace.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"

void UNightSchoolSpriteWidget::SetTexture(UTexture2D* InTexture)
{
	Texture = InTexture;
	Brush.SetResourceObject(Texture);
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = Texture ? FVector2D(Texture->GetSizeX(), Texture->GetSizeY()) : FVector2D(32.f, 32.f);
	Brush.TintColor = FSlateColor(FLinearColor(0.92f, 0.88f, 0.82f));
}

TSharedRef<SWidget> UNightSchoolSpriteWidget::RebuildWidget()
{
	SetTexture(Texture);
	return SNew(SImage).Image(&Brush);
}

namespace NightSchoolArt
{
	namespace
	{
		UStaticMesh* LoadMesh(const TCHAR* Path)
		{
			return LoadObject<UStaticMesh>(nullptr, Path);
		}

		float Hash01(int32 X, int32 Y)
		{
			int32 N = X * 374761393 + Y * 668265263;
			N = (N ^ (N >> 13)) * 1274126177;
			return static_cast<float>(N & 0x7fffffff) / static_cast<float>(0x7fffffff);
		}

		float Noise(float X, float Y)
		{
			const int32 X0 = FMath::FloorToInt(X);
			const int32 Y0 = FMath::FloorToInt(Y);
			const float Tx = FMath::SmoothStep(0.f, 1.f, X - static_cast<float>(X0));
			const float Ty = FMath::SmoothStep(0.f, 1.f, Y - static_cast<float>(Y0));
			const float A = Hash01(X0, Y0);
			const float B = Hash01(X0 + 1, Y0);
			const float C = Hash01(X0, Y0 + 1);
			const float D = Hash01(X0 + 1, Y0 + 1);
			return FMath::Lerp(FMath::Lerp(A, B, Tx), FMath::Lerp(C, D, Tx), Ty);
		}

		void Put(TArray<FColor>& Pixels, int32 Width, int32 Height, int32 X, int32 Y, const FColor& Color)
		{
			if (X < 0 || Y < 0 || X >= Width || Y >= Height)
			{
				return;
			}
			Pixels[Y * Width + X] = Color;
		}

		void FillEllipse(TArray<FColor>& Pixels, int32 Width, int32 Height, float CX, float CY, float RX, float RY, const FColor& Color, bool bSoft)
		{
			const int32 MinX = FMath::Max(0, FMath::FloorToInt(CX - RX - 1.f));
			const int32 MaxX = FMath::Min(Width - 1, FMath::CeilToInt(CX + RX + 1.f));
			const int32 MinY = FMath::Max(0, FMath::FloorToInt(CY - RY - 1.f));
			const int32 MaxY = FMath::Min(Height - 1, FMath::CeilToInt(CY + RY + 1.f));
			for (int32 Y = MinY; Y <= MaxY; ++Y)
			{
				for (int32 X = MinX; X <= MaxX; ++X)
				{
					const float NX = (static_cast<float>(X) - CX) / RX;
					const float NY = (static_cast<float>(Y) - CY) / RY;
					const float D = NX * NX + NY * NY;
					if (D <= 1.f)
					{
						FColor Out = Color;
						if (bSoft)
						{
							const float Edge = FMath::Clamp((1.f - D) * 4.f, 0.f, 1.f);
							const FColor Under = Pixels[Y * Width + X];
							Out.R = static_cast<uint8>(FMath::Lerp(static_cast<float>(Under.R), static_cast<float>(Color.R), Edge));
							Out.G = static_cast<uint8>(FMath::Lerp(static_cast<float>(Under.G), static_cast<float>(Color.G), Edge));
							Out.B = static_cast<uint8>(FMath::Lerp(static_cast<float>(Under.B), static_cast<float>(Color.B), Edge));
						}
						Put(Pixels, Width, Height, X, Y, Out);
					}
				}
			}
		}
	}

	UStaticMesh* Cube()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
		}
		return Cached.Get();
	}

	UStaticMesh* Sphere()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		}
		return Cached.Get();
	}

	UStaticMesh* Cylinder()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		}
		return Cached.Get();
	}

	UStaticMesh* Cone()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
		}
		return Cached.Get();
	}

	UMaterialInstanceDynamic* MakeColor(UObject* Outer, const FLinearColor& Color)
	{
		UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (!Base || !Outer)
		{
			return nullptr;
		}
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, Outer);
		if (Mid)
		{
			Mid->SetVectorParameterValue(TEXT("Color"), Color);
		}
		return Mid;
	}

	UFont* GetRuntimeFont()
	{
		static TWeakObjectPtr<UFont> Cached;
		if (Cached.IsValid())
		{
			return Cached.Get();
		}

		const FString SourcePath = FPaths::ProjectContentDir() / TEXT("NightSchool/Fonts/DroidSansFallbackFull.ttf");
		const FString DestPath = FPaths::ProjectSavedDir() / TEXT("NightSchool/DroidSansFallbackFull.ttf");
		TArray<uint8> Bytes;
		if (!FFileHelper::LoadFileToArray(Bytes, *SourcePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("里屋：找不到中文字体 %s"), *SourcePath);
			return nullptr;
		}
		FFileHelper::SaveArrayToFile(Bytes, *DestPath);

		UFontFace* Face = NewObject<UFontFace>(GetTransientPackage(), NAME_None, RF_Transient);
		Face->SourceFilename = FPaths::ConvertRelativePathToFull(DestPath);
		Face->Hinting = EFontHinting::Auto;
		Face->LoadingPolicy = EFontLoadingPolicy::LazyLoad;

		UFont* Font = NewObject<UFont>(GetTransientPackage(), NAME_None, RF_Transient);
		Font->FontCacheType = EFontCacheType::Runtime;
		FTypefaceEntry& Entry = Font->CompositeFont.DefaultTypeface.Fonts.AddDefaulted_GetRef();
		Entry.Name = FName(TEXT("Regular"));
		Entry.Font = FFontData(Face);
		Cached = Font;
		return Font;
	}

	FSlateFontInfo FontInfo(float Size)
	{
		if (UFont* Font = GetRuntimeFont())
		{
			return FSlateFontInfo(Font, Size, FName(TEXT("Regular")));
		}
		FSlateFontInfo Fallback = FCoreStyle::Get().GetFontStyle("NormalFont");
		Fallback.Size = Size;
		return Fallback;
	}

	UTexture2D* CreateTexture(UObject* Outer, int32 Width, int32 Height, const TArray<FColor>& Pixels)
	{
		if (!Outer || Width <= 0 || Height <= 0 || Pixels.Num() != Width * Height)
		{
			return nullptr;
		}

		UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
		if (!Texture)
		{
			return nullptr;
		}
		Texture->SRGB = true;
		Texture->NeverStream = true;
		Texture->Filter = TF_Bilinear;
		Texture->AddressX = TA_Clamp;
		Texture->AddressY = TA_Clamp;

		FTexturePlatformData* PlatformData = Texture->GetPlatformData();
		if (!PlatformData || PlatformData->Mips.Num() == 0)
		{
			return nullptr;
		}

		FTexture2DMipMap& Mip = PlatformData->Mips[0];
		if (Mip.BulkData.GetBulkDataSize() < Pixels.Num() * static_cast<int64>(sizeof(FColor)))
		{
			return nullptr;
		}
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		if (!Data)
		{
			return nullptr;
		}
		FMemory::Memcpy(Data, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
		Mip.BulkData.Unlock();
		Texture->UpdateResource();
		return Texture;
	}

	void FillRect(TArray<FColor>& Pixels, int32 Width, int32 Height, int32 X0, int32 Y0, int32 X1, int32 Y1, const FColor& Color)
	{
		for (int32 Y = Y0; Y <= Y1; ++Y)
		{
			for (int32 X = X0; X <= X1; ++X)
			{
				Put(Pixels, Width, Height, X, Y, Color);
			}
		}
	}

	UTexture2D* MakeMirror(UObject* Outer)
	{
		constexpr int32 W = 256;
		constexpr int32 H = 336;
		TArray<FColor> Pixels;
		Pixels.SetNum(W * H);

		for (int32 Y = 0; Y < H; ++Y)
		{
			for (int32 X = 0; X < W; ++X)
			{
				const float N = Noise(X * 0.07f, Y * 0.05f);
				const float Vignette = FMath::Clamp(1.f - FVector2D(X - 128.f, Y - 168.f).Size() / 210.f, 0.f, 1.f);
				const uint8 Channel = static_cast<uint8>(FMath::Clamp(16.f + N * 18.f + Vignette * 22.f, 0.f, 255.f));
				Pixels[Y * W + X] = FColor(Channel, Channel + 4, Channel + 12, 255);
			}
		}

		FillRect(Pixels, W, H, 0, 0, W - 1, 14, FColor(28, 26, 24, 255));
		FillRect(Pixels, W, H, 0, H - 15, W - 1, H - 1, FColor(28, 26, 24, 255));
		FillRect(Pixels, W, H, 0, 0, 14, H - 1, FColor(28, 26, 24, 255));
		FillRect(Pixels, W, H, W - 15, 0, W - 1, H - 1, FColor(28, 26, 24, 255));

		FillEllipse(Pixels, W, H, 128.f, 188.f, 36.f, 90.f, FColor(58, 66, 74, 255), true);

		for (int32 Step = 0; Step < 240; ++Step)
		{
			const int32 X = 36 + Step;
			const int32 Y = 28 + (Step * 270) / 240 + ((Step / 11) % 3);
			const FColor Crack(186, 196, 204, 255);
			Put(Pixels, W, H, X, Y, Crack);
			Put(Pixels, W, H, X, Y + 1, FColor(120, 132, 142, 255));
			if (Step > 90 && Step < 150)
			{
				Put(Pixels, W, H, X - (Step - 90) / 2, Y + 8, Crack);
			}
		}
		for (int32 Step = 0; Step < 120; ++Step)
		{
			const int32 X = 180 - Step / 2;
			const int32 Y = 70 + Step;
			Put(Pixels, W, H, X, Y, FColor(150, 162, 170, 255));
		}

		FillEllipse(Pixels, W, H, 214.f, 42.f, 11.f, 11.f, FColor(120, 16, 14, 255), false);
		FillEllipse(Pixels, W, H, 214.f, 42.f, 4.f, 4.f, FColor(255, 70, 54, 255), false);
		return CreateTexture(Outer, W, H, Pixels);
	}

	UTexture2D* MakeScoreSheet(UObject* Outer)
	{
		constexpr int32 W = 256;
		constexpr int32 H = 320;
		TArray<FColor> Pixels;
		Pixels.SetNum(W * H);
		for (int32 Y = 0; Y < H; ++Y)
		{
			for (int32 X = 0; X < W; ++X)
			{
				const float N = Noise(X * 0.08f, Y * 0.04f);
				Pixels[Y * W + X] = FColor(
					static_cast<uint8>(226 + N * 16.f),
					static_cast<uint8>(222 + N * 12.f),
					static_cast<uint8>(206 + N * 10.f),
					255);
			}
		}

		FillRect(Pixels, W, H, 0, 0, W - 1, 48, FColor(36, 58, 92, 255));
		for (int32 Row = 0; Row < 7; ++Row)
		{
			const int32 Y = 72 + Row * 28;
			FillRect(Pixels, W, H, 18, Y, W - 19, Y + 1, FColor(170, 176, 184, 255));
			FillRect(Pixels, W, H, 28, Y + 8, 120, Y + 14, FColor(196, 198, 202, 255));
		}
		FillRect(Pixels, W, H, 148, 78, 149, 250, FColor(186, 190, 196, 255));

		const FColor Ink(176, 28, 32, 255);
		FillRect(Pixels, W, H, 168, 214, 206, 222, Ink);
		FillRect(Pixels, W, H, 168, 214, 176, 246, Ink);
		FillRect(Pixels, W, H, 168, 246, 206, 254, Ink);
		FillRect(Pixels, W, H, 198, 254, 206, 286, Ink);
		FillRect(Pixels, W, H, 168, 278, 206, 286, Ink);
		FillRect(Pixels, W, H, 222, 214, 232, 286, Ink);
		return CreateTexture(Outer, W, H, Pixels);
	}
}
