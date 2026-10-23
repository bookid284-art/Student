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
			UE_LOG(LogTemp, Warning, TEXT("回课：找不到中文字体 %s"), *SourcePath);
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

	UTexture2D* MakePortrait(UObject* Outer)
	{
		constexpr int32 W = 256;
		constexpr int32 H = 336;
		TArray<FColor> Pixels;
		Pixels.SetNum(W * H);

		for (int32 Y = 0; Y < H; ++Y)
		{
			for (int32 X = 0; X < W; ++X)
			{
				const float N = Noise(X * 0.08f, Y * 0.08f);
				const float Fiber = Noise(X * 0.4f, Y * 0.05f);
				const uint8 R = static_cast<uint8>(FMath::Clamp(168.f + N * 28.f + Fiber * 10.f, 0.f, 255.f));
				const uint8 G = static_cast<uint8>(FMath::Clamp(150.f + N * 22.f, 0.f, 255.f));
				const uint8 B = static_cast<uint8>(FMath::Clamp(118.f + N * 16.f, 0.f, 255.f));
				Pixels[Y * W + X] = FColor(R, G, B, 255);
			}
		}

		for (int32 X = 12; X < W - 12; ++X)
		{
			for (int32 T = 0; T < 4; ++T)
			{
				Put(Pixels, W, H, X, 12 + T, FColor(92, 28, 24, 255));
				Put(Pixels, W, H, X, H - 16 + T, FColor(92, 28, 24, 255));
			}
		}
		for (int32 Y = 12; Y < H - 12; ++Y)
		{
			for (int32 T = 0; T < 4; ++T)
			{
				Put(Pixels, W, H, 12 + T, Y, FColor(92, 28, 24, 255));
				Put(Pixels, W, H, W - 16 + T, Y, FColor(92, 28, 24, 255));
			}
		}

		FillEllipse(Pixels, W, H, 128.f, 118.f, 78.f, 58.f, FColor(18, 16, 18, 255), false);
		FillEllipse(Pixels, W, H, 78.f, 150.f, 28.f, 70.f, FColor(22, 18, 20, 255), true);
		FillEllipse(Pixels, W, H, 178.f, 150.f, 28.f, 70.f, FColor(22, 18, 20, 255), true);
		FillEllipse(Pixels, W, H, 128.f, 168.f, 52.f, 64.f, FColor(186, 156, 132, 255), false);
		FillEllipse(Pixels, W, H, 128.f, 196.f, 28.f, 18.f, FColor(168, 132, 112, 255), true);

		for (int32 X = 96; X < 118; ++X)
		{
			Put(Pixels, W, H, X, 158 + ((X / 3) % 2), FColor(42, 28, 26, 255));
		}
		for (int32 X = 140; X < 162; ++X)
		{
			Put(Pixels, W, H, X, 158 + ((X / 3) % 2), FColor(42, 28, 26, 255));
		}
		FillEllipse(Pixels, W, H, 108.f, 172.f, 8.f, 4.f, FColor(36, 24, 22, 255), false);
		FillEllipse(Pixels, W, H, 150.f, 172.f, 8.f, 4.f, FColor(36, 24, 22, 255), false);
		FillEllipse(Pixels, W, H, 110.f, 171.f, 2.f, 2.f, FColor(230, 220, 210, 255), false);
		FillEllipse(Pixels, W, H, 152.f, 171.f, 2.f, 2.f, FColor(230, 220, 210, 255), false);

		for (int32 Y = 176; Y < 196; ++Y)
		{
			Put(Pixels, W, H, 128, Y, FColor(120, 82, 70, 255));
		}
		for (int32 X = 118; X < 140; ++X)
		{
			Put(Pixels, W, H, X, 214, FColor(92, 48, 46, 255));
		}

		FillEllipse(Pixels, W, H, 128.f, 268.f, 70.f, 48.f, FColor(36, 34, 40, 255), false);
		for (int32 Y = 230; Y < 250; ++Y)
		{
			for (int32 X = 112; X < 146; ++X)
			{
				Put(Pixels, W, H, X, Y, FColor(214, 206, 190, 255));
			}
		}

		FillEllipse(Pixels, W, H, 196.f, 286.f, 22.f, 22.f, FColor(148, 28, 24, 255), false);
		FillEllipse(Pixels, W, H, 196.f, 286.f, 10.f, 10.f, FColor(176, 154, 120, 255), false);

		for (int32 I = 0; I < 40; ++I)
		{
			const int32 X = 28 + (I * 17) % 12;
			const int32 Y = 70 + I * 5;
			if (Y < H - 30)
			{
				Put(Pixels, W, H, X, Y, FColor(48, 36, 32, 255));
				Put(Pixels, W, H, X + 1, Y, FColor(48, 36, 32, 255));
			}
		}

		return CreateTexture(Outer, W, H, Pixels);
	}

	UTexture2D* MakeRegisterCover(UObject* Outer)
	{
		constexpr int32 W = 256;
		constexpr int32 H = 320;
		TArray<FColor> Pixels;
		Pixels.SetNum(W * H);
		for (int32 Y = 0; Y < H; ++Y)
		{
			for (int32 X = 0; X < W; ++X)
			{
				const float N = Noise(X * 0.06f, Y * 0.06f);
				const bool bStripe = X > 28 && X < 52;
				FColor Cloth = bStripe ? FColor(120, 24, 22, 255) : FColor(28, 36, 48, 255);
				Cloth.R = static_cast<uint8>(FMath::Clamp(Cloth.R + N * 18.f, 0.f, 255.f));
				Cloth.G = static_cast<uint8>(FMath::Clamp(Cloth.G + N * 12.f, 0.f, 255.f));
				Cloth.B = static_cast<uint8>(FMath::Clamp(Cloth.B + N * 10.f, 0.f, 255.f));
				const float Burn = (X + Y * 0.35f) / static_cast<float>(W);
				if (Burn > 1.15f && Noise(X * 0.1f, Y * 0.1f) > 0.25f)
				{
					Cloth = FColor(12, 10, 10, 255);
				}
				Pixels[Y * W + X] = Cloth;
			}
		}
		for (int32 Y = 78; Y < 250; ++Y)
		{
			for (int32 X = 78; X < 210; ++X)
			{
				const float N = Noise(X * 0.2f, Y * 0.2f);
				Pixels[Y * W + X] = FColor(
					static_cast<uint8>(150 + N * 20.f),
					static_cast<uint8>(132 + N * 16.f),
					static_cast<uint8>(96 + N * 10.f),
					255);
			}
		}
		return CreateTexture(Outer, W, H, Pixels);
	}
}
