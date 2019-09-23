
#include "AnimatedGIFDecoder.h"

#define GIF_MGET(m,s,a,c) if (c) m = (uint8*)FMemory::Malloc(s); else FMemory::Free(m);
#include "gif_load/gif_load.h" // from: https://github.com/hidefromkgb/gif_load

extern "C"
{
	void GIFFrameLoader(void *data, struct GIF_WHDR *whdr) {
		UAnimatedGIFDecoder* OutGIF = (UAnimatedGIFDecoder*)data;

		//-- init on first frame
		if (OutGIF->GetFrameCount() == 0) {
			OutGIF->Import_Init(whdr->xdim, whdr->ydim, whdr->bkgd, whdr->nfrm);
		}

		//-- import frame
		int FrameIndex = whdr->ifrm;

		check(OutGIF->GetFrameCount() == whdr->nfrm);
		check(FrameIndex >= 0 && FrameIndex < OutGIF->GetFrameCount());

		FGIFFrame& Frame = OutGIF->GetFrame(FrameIndex);

		//-- copy properties
		Frame.Time = whdr->time*0.01f;	// 1 GIF time units = 10 msec
		Frame.Index = whdr->ifrm;
		Frame.Width = whdr->frxd;
		Frame.Height = whdr->fryd;
		Frame.OffsetX = whdr->frxo;
		Frame.OffsetY = whdr->fryo;
		Frame.Interlacing = whdr->intr;
		Frame.Mode = whdr->mode;
		Frame.TransparentIndex = whdr->tran;

		//-- copy pixel data
		int NumPixel = Frame.Width*Frame.Height;
		Frame.PixelIndices.Init(0, NumPixel);
		FMemory::Memcpy(Frame.PixelIndices.GetData(), whdr->bptr, NumPixel);

		//-- copy pal
		int PaletteSize = whdr->clrs;
		Frame.Palette.Init(FColor::Black, PaletteSize);
		for (int i = 0; i < PaletteSize; i++) {
			FColor& uc = Frame.Palette[i];
			uc.R = whdr->cpal[i].R;
			uc.G = whdr->cpal[i].G;
			uc.B = whdr->cpal[i].B;
			uc.A = 255;
		}// end of for
	}
}// end of "C"

void LoadGIFBinary(UAnimatedGIFDecoder* OutGIF, const uint8 * Buffer, uint32 BufferSize)
{
	GIF_Load((void*)Buffer, BufferSize, GIFFrameLoader, 0, (void*)OutGIF, 0L);
}