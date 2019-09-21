#ifndef ULPAPANeoPixelBus_h
#define ULPAPANeoPixelBus_h

#include "ULPAPADriver.h"
#include "NeoPixelBus.h"
#include "internal/RgbColor.h"


class ULPAPAMethod
{
    public:

    ULPAPAMethod(uint8_t pinClock, uint8_t pinData, void* apaRtcPtr, uint16_t pixelCount, size_t elementSize) :
        ulp((ulp_apa_led_t *)apaRtcPtr,pixelCount),
        _pixels((uint8_t *)apaRtcPtr),
        _sizePixels(pixelCount * elementSize)
    {
        ulp.begin(pinData,pinClock);
    }

    ~ULPAPAMethod()
    {
    }

    bool IsReadyToUpdate() const
    {
        return true;
    }

    void Initialize()
    {
		ulp.setBrightness(255);
    }

    void Update(bool)
    {
        ulp.show();
    }

    uint8_t* getPixels() const
    {
        return _pixels;
    };

    size_t getPixelsSize() const
    {
        return _sizePixels;
    };

private:
    ULPAPADriver ulp;
    uint8_t* _pixels;
    const size_t   _sizePixels;
};


class ULPAPAElements
{
public:
    static const size_t PixelSize = sizeof(ulp_apa_led_t);

    static uint8_t* getPixelAddress(uint8_t* pPixels, uint16_t indexPixel)
    {
        return pPixels + indexPixel * PixelSize;
    }
    static const uint8_t* getPixelAddress(const uint8_t* pPixels, uint16_t indexPixel)
    {
        return pPixels + indexPixel * PixelSize;
    }

    static void replicatePixel(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        ulp_apa_led_t* pEnd = (ulp_apa_led_t*)pPixelDest + (count * PixelSize);
        ulp_apa_led_t* ptrDest = (ulp_apa_led_t*)pPixelDest;
        ulp_apa_led_t* ptrSrc = (ulp_apa_led_t*)pPixelSrc;
        while (ptrDest < pEnd)
        {
            ptrDest->rgb(ptrSrc->rgb());
            ++ptrDest;
        }
    }

    static void movePixelsInc(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        ulp_apa_led_t* pEnd = (ulp_apa_led_t*)pPixelDest + (count * PixelSize);
        ulp_apa_led_t* ptrDest = (ulp_apa_led_t*)pPixelDest;
        ulp_apa_led_t* ptrSrc = (ulp_apa_led_t*)pPixelSrc;
        while (ptrDest < pEnd)
        {
            ptrDest->rgb(ptrSrc->rgb());
            ++ptrDest;
            ++ptrSrc;
        }
    }

    static void movePixelsInc_P(uint8_t* pPixelDest, PGM_VOID_P pPixelSrc, uint16_t count)
    {
        movePixelsInc(pPixelDest, (const uint8_t*) pPixelSrc, count);
    }

    static void movePixelsDec(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        ulp_apa_led_t* ptrDest = (ulp_apa_led_t*)pPixelDest + (count * PixelSize);
        ulp_apa_led_t* ptrSrc = (ulp_apa_led_t*)pPixelSrc + (count * PixelSize);
        while (ptrDest > (ulp_apa_led_t*)pPixelDest)
        {
            ptrDest->rgb(ptrSrc->rgb());
            --ptrDest;
            --ptrSrc;
        }
    }

    typedef RgbColor ColorObject;
};


class ULPAPARgbFeature : public ULPAPAElements
{
public:
    static void applyPixelColor(uint8_t* pPixels, uint16_t indexPixel, ColorObject color)
    {
        ulp_apa_led_t* p = (ulp_apa_led_t*)getPixelAddress(pPixels, indexPixel);

        p->rgb((color.R << 16) | (color.G << 8) | color.B);
    }

    static ColorObject retrievePixelColor(uint8_t* pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        ulp_apa_led_t* p = (ulp_apa_led_t*)getPixelAddress(pPixels, indexPixel);

        color.G = p->green();
        color.R = p->red();
        color.B = p->blue();

        return color;
    }

    static ColorObject retrievePixelColor_P(PGM_VOID_P pPixels, uint16_t indexPixel)
    {
        return retrievePixelColor((uint8_t*) pPixels, indexPixel);
    }

};


#endif