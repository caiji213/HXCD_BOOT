#ifndef	__CONVERT_INT_BYTESARRAY_H__
#define	__CONVERT_INT_BYTESARRAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Uint16 *************************************/
/* BigEndian */
unsigned short BigEndianBytesArray_To_Uint16(const unsigned char * pdata);
void Uint16_To_BigEndianBytesArray(unsigned char * pdata, unsigned short value);
/* LittleEndian */
unsigned short LittleEndianBytesArray_To_Uint16(const unsigned char * pdata);
void Uint16_To_LittleEndianBytesArray(unsigned char * pdata, unsigned short value);

/***************************** Sint16 *************************************/
/* BigEndian */
signed short BigEndianBytesArray_To_Sint16(const unsigned char * pdata);
void Sint16_To_BigEndianBytesArray(unsigned char * pdata, signed short value);
/* LittleEndian */
signed short LittleEndianBytesArray_To_Sint16(const unsigned char * pdata);
void Sint16_To_LittleEndianBytesArray(unsigned char * pdata, signed short value);

/***************************** Uint32 *************************************/
/* BigEndian */
unsigned long BigEndianBytesArray_To_Uint32(const unsigned char * pdata);
void Uint32_To_BigEndianBytesArray(unsigned char * pdata, unsigned long value);
/* LittleEndian */
unsigned long LittleEndianBytesArray_To_Uint32(const unsigned char * pdata);
void Uint32_To_LittleEndianBytesArray(unsigned char * pdata, unsigned long value);

/***************************** Sint32 *************************************/
/* BigEndian */
signed long BigEndianBytesArray_To_Sint32(const unsigned char * pdata);
void Sint32_To_BigEndianBytesArray(unsigned char * pdata, signed long value);
/* LittleEndian */
signed long LittleEndianBytesArray_To_Sint32(const unsigned char * pdata);
void Sint32_To_LittleEndianBytesArray(unsigned char * pdata, signed long value);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
