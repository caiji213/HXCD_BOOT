#include "User_Convert_Int_BytesArray.h"

/***************************** Uint16 *************************************/
/* BigEndian */
unsigned short BigEndianBytesArray_To_Uint16(const unsigned char *pdata)
{
    unsigned long ret = 0;
    unsigned long v;
    int n;
    for (n = 0; n < 2; n++)
    {
        v = pdata[n];
        ret |= v << (8 - n * 8);
    }
    return ret;
}
void Uint16_To_BigEndianBytesArray(unsigned char *pdata, unsigned short value)
{
    int n;
    for (n = 0; n < 2; n++)
    {
        pdata[n] = (value >> (8 - n * 8)) & 0xFF;
    }
}

/* LittleEndian */
unsigned short LittleEndianBytesArray_To_Uint16(const unsigned char *pdata)
{
    unsigned long ret = 0;
    unsigned long v;
    int n;
    for (n = 0; n < 2; n++)
    {
        v = pdata[n];
        ret |= v << (n * 8);
    }
    return ret;
}
void Uint16_To_LittleEndianBytesArray(unsigned char *pdata, unsigned short value)
{
    int n;
    for (n = 0; n < 2; n++)
    {
        pdata[n] = (value >> (n * 8)) & 0xFF;
    }
}

/***************************** Sint16 *************************************/
/* BigEndian */
signed short BigEndianBytesArray_To_Sint16(const unsigned char *pdata)
{
    signed long ret = 0;
    signed long v;
    int n;
    for (n = 0; n < 2; n++)
    {
        v = pdata[n];
        ret |= v << (8 - n * 8);
    }
    return ret;
}

void Sint16_To_BigEndianBytesArray(unsigned char *pdata, signed short value)
{
    int n;
    for (n = 0; n < 2; n++)
    {
        pdata[n] = (value >> (8 - n * 8)) & 0xFF;
    }
}

/* LittleEndian */
signed short LittleEndianBytesArray_To_Sint16(const unsigned char *pdata)
{
    signed long ret = 0;
    signed long v;
    int n;
    for (n = 0; n < 2; n++)
    {
        v = pdata[n];
        ret |= v << (n * 8);
    }
    return ret;
}

void Sint16_To_LittleEndianBytesArray(unsigned char *pdata, signed short value)
{
    int n;
    for (n = 0; n < 2; n++)
    {
        pdata[n] = (value >> (n * 8)) & 0xFF;
    }
}

/***************************** Uint32 *************************************/
/* BigEndian */
unsigned long BigEndianBytesArray_To_Uint32(const unsigned char *pdata)
{
    unsigned long ret = 0;
    unsigned long v;
    int n;
    for (n = 0; n < 4; n++)
    {
        v = pdata[n];
        ret |= v << ((3 - n) * 8);
    }
    return ret;
}

void Uint32_To_BigEndianBytesArray(unsigned char *pdata, unsigned long value)
{
    int n;
    for (n = 0; n < 4; n++)
    {
        pdata[n] = (value >> ((3 - n) * 8)) & 0xFF;
    }
}

/* LittleEndian */
unsigned long LittleEndianBytesArray_To_Uint32(const unsigned char *pdata)
{
    unsigned long ret = 0;
    unsigned long v;
    int n;
    for (n = 0; n < 4; n++)
    {
        v = pdata[n];
        ret |= v << (n * 8);
    }
    return ret;
}

void Uint32_To_LittleEndianBytesArray(unsigned char *pdata, unsigned long value)
{
    int n;
    for (n = 0; n < 4; n++)
    {
        pdata[n] = (value >> (n * 8)) & 0xFF;
    }
}

/***************************** Sint32 *************************************/
/* BigEndian */
signed long BigEndianBytesArray_To_Sint32(const unsigned char *pdata)
{
    signed long ret = 0;
    signed long v;
    int n;
    for (n = 0; n < 4; n++)
    {
        v = pdata[n];
        ret |= v << ((3 - n) * 8);
    }
    return ret;
}
void Sint32_To_BigEndianBytesArray(unsigned char *pdata, signed long value)
{
    int n;
    for (n = 0; n < 4; n++)
    {
        pdata[n] = (value >> ((3 - n) * 8)) & 0xFF;
    }
}

/* LittleEndian */
signed long LittleEndianBytesArray_To_Sint32(const unsigned char *pdata)
{
    unsigned long ret = 0;
    unsigned long v;
    int n;
    for (n = 0; n < 4; n++)
    {
        v = pdata[n];
        ret |= v << (n * 8);
    }
    return ret;
}
void Sint32_To_LittleEndianBytesArray(unsigned char *pdata, signed long value)
{
    int n;
    for (n = 0; n < 4; n++)
    {
        pdata[n] = (value >> (n * 8)) & 0xFF;
    }
}
