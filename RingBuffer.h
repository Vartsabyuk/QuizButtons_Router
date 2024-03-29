/** \file
 *  \brief Lightweight ring (circular) buffer, for fast insertion/deletion of bytes.
 *
 *  Lightweight ring buffer, for fast insertion/deletion. Multiple buffers can be created of
 *  different sizes to suit different needs.
 *
 *  Note that for each buffer, insertion and removal operations may occur at the same time (via
 *  a multi-threaded ISR based system) however the same kind of operation (two or more insertions
 *  or deletions) must not overlap. If there is possibility of two or more of the same kind of
 *  operating occurring at the same point in time, atomic (mutex) locking should be used.
 */

/** \ingroup Group_MiscDrivers
 *  \defgroup Group_RingBuff Generic Byte Ring Buffer - LUFA/Drivers/Misc/RingBuffer.h
 *  \brief Lightweight ring buffer, for fast insertion/deletion of bytes.
 *
 *  \section Sec_RingBuff_Dependencies Module Source Dependencies
 *  The following files must be built with any user project that uses this module:
 *    - None
 *
 *  \section Sec_RingBuff_ModDescription Module Description
 *  Lightweight ring buffer, for fast insertion/deletion. Multiple buffers can be created of
 *  different sizes to suit different needs.
 *
 *  Note that for each buffer, insertion and removal operations may occur at the same time (via
 *  a multi-threaded ISR based system) however the same kind of operation (two or more insertions
 *  or deletions) must not overlap. If there is possibility of two or more of the same kind of
 *  operating occurring at the same point in time, atomic (mutex) locking should be used.
 *
 *  \section Sec_RingBuff_ExampleUsage Example Usage
 *  The following snippet is an example of how this module may be used within a typical
 *  application.
 *
 *  \code
 *      // Create the buffer structure and its underlying storage array
 *      RingBuffer_t Buffer;
 *      uint8_t      BufferData[128];
 *
 *      // Initialize the buffer with the created storage array
 *      RingBuffer_InitBuffer(&Buffer, BufferData, sizeof(BufferData));
 *
 *      // Insert some data into the buffer
 *      RingBuffer_Insert(&Buffer, 'H');
 *      RingBuffer_Insert(&Buffer, 'E');
 *      RingBuffer_Insert(&Buffer, 'L');
 *      RingBuffer_Insert(&Buffer, 'L');
 *      RingBuffer_Insert(&Buffer, 'O');
 *
 *      // Cache the number of stored bytes in the buffer
 *      uint16_t BufferCount = RingBuffer_GetCount(&Buffer);
 *
 *      // Printer stored data length
 *      printf("Buffer Length: %d, Buffer Data: \r\n", BufferCount);
 *
 *      // Print contents of the buffer one character at a time
 *      while (BufferCount--)
 *        putc(RingBuffer_Remove(&Buffer));
 *  \endcode
 *
 *  @{
 */

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

	/* Includes: */
		#include <HAL.h>

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Type Defines: */
		/** \brief Ring Buffer Management Structure.
		 *
		 *  Type define for a new ring buffer object. Buffers should be initialized via a call to
		 *  \ref RingBuffer_InitBuffer() before use.
		 */
		typedef struct
		{
			u08* In; /**< Current storage location in the circular buffer. */
			u08* Out; /**< Current retrieval location in the circular buffer. */
			u08* Start; /**< Pointer to the start of the buffer's underlying storage array. */
			u08* End; /**< Pointer to the end of the buffer's underlying storage array. */
			u16 Size; /**< Size of the buffer's underlying storage array. */
			u16 Count; /**< Number of bytes currently stored in the buffer. */
		} RingBuffer_t;

	/* Inline Functions: */
		/** Initializes a ring buffer ready for use. Buffers must be initialized via this function
		 *  before any operations are called upon them. Already initialized buffers may be reset
		 *  by re-initializing them using this function.
		 *
		 *  \param[out] Buffer   Pointer to a ring buffer structure to initialize.
		 *  \param[out] DataPtr  Pointer to a global array that will hold the data stored into the ring buffer.
		 *  \param[out] Size     Maximum number of bytes that can be stored in the underlying data array.
		 */
		static inline void RingBuffer_InitBuffer(RingBuffer_t* Buffer,
		                                         u08* const DataPtr,
		                                         const u16 Size);
		static inline void RingBuffer_InitBuffer(RingBuffer_t* Buffer,
		                                         u08* const DataPtr,
		                                         const u16 Size)
		{
			u08	nointerrupted = 0;
			if (STATUS_REG & (1<<Interrupt_Flag))  // Если прерывания разрешены, то запрещаем их.
			{
				Disable_Interrupt
				nointerrupted = 1;					// И ставим флаг, что мы не в прерывании. 
			}

			Buffer->In     = DataPtr;
			Buffer->Out    = DataPtr;
			Buffer->Start  = &DataPtr[0];
			Buffer->End    = &DataPtr[Size];
			Buffer->Size   = Size;
			Buffer->Count  = 0;

			if (nointerrupted) Enable_Interrupt	
		}

		/** Retrieves the current number of bytes stored in a particular buffer. This value is computed
		 *  by entering an atomic lock on the buffer, so that the buffer cannot be modified while the
		 *  computation takes place. This value should be cached when reading out the contents of the buffer,
		 *  so that as small a time as possible is spent in an atomic lock.
		 *
		 *  \note The value returned by this function is guaranteed to only be the minimum number of bytes
		 *        stored in the given buffer; this value may change as other threads write new data, thus
		 *        the returned number should be used only to determine how many successive reads may safely
		 *        be performed on the buffer.
		 *
		 *  \param[in] Buffer  Pointer to a ring buffer structure whose count is to be computed.
		 *
		 *  \return Number of bytes currently stored in the buffer.
		 */
		static inline u16 RingBuffer_GetCount(RingBuffer_t* const Buffer);
		static inline u16 RingBuffer_GetCount(RingBuffer_t* const Buffer)
		{
			u16 Count;
			u08	nointerrupted = 0;
			if (STATUS_REG & (1<<Interrupt_Flag))  // Если прерывания разрешены, то запрещаем их.
			{
				Disable_Interrupt
				nointerrupted = 1;					// И ставим флаг, что мы не в прерывании. 
			}

			Count = Buffer->Count;

			if (nointerrupted) Enable_Interrupt	
			return Count;
		}

		/** Retrieves the free space in a particular buffer. This value is computed by entering an atomic lock
		 *  on the buffer, so that the buffer cannot be modified while the computation takes place.
		 *
		 *  \note The value returned by this function is guaranteed to only be the maximum number of bytes
		 *        free in the given buffer; this value may change as other threads write new data, thus
		 *        the returned number should be used only to determine how many successive writes may safely
		 *        be performed on the buffer when there is a single writer thread.
		 *
		 *  \param[in] Buffer  Pointer to a ring buffer structure whose free count is to be computed.
		 *
		 *  \return Number of free bytes in the buffer.
		 */
		static inline u16 RingBuffer_GetFreeCount(RingBuffer_t* const Buffer);
		static inline u16 RingBuffer_GetFreeCount(RingBuffer_t* const Buffer)
		{
			return (Buffer->Size - RingBuffer_GetCount(Buffer));
		}

		/** Atomically determines if the specified ring buffer contains any data. This should
		 *  be tested before removing data from the buffer, to ensure that the buffer does not
		 *  underflow.
		 *
		 *  If the data is to be removed in a loop, store the total number of bytes stored in the
		 *  buffer (via a call to the \ref RingBuffer_GetCount() function) in a temporary variable
		 *  to reduce the time spent in atomicity locks.
		 *
		 *  \param[in,out] Buffer  Pointer to a ring buffer structure to insert into.
		 *
		 *  \return Boolean \c true if the buffer contains no free space, \c false otherwise.
		 */
		static inline BOOL RingBuffer_IsEmpty(RingBuffer_t* const Buffer);
		static inline BOOL RingBuffer_IsEmpty(RingBuffer_t* const Buffer)
		{
			return (RingBuffer_GetCount(Buffer) == 0);
		}

		/** Atomically determines if the specified ring buffer contains any free space. This should
		 *  be tested before storing data to the buffer, to ensure that no data is lost due to a
		 *  buffer overrun.
		 *
		 *  \param[in,out] Buffer  Pointer to a ring buffer structure to insert into.
		 *
		 *  \return Boolean \c true if the buffer contains no free space, \c false otherwise.
		 */
		static inline BOOL RingBuffer_IsFull(RingBuffer_t* const Buffer);
		static inline BOOL RingBuffer_IsFull(RingBuffer_t* const Buffer)
		{
			return (RingBuffer_GetCount(Buffer) == Buffer->Size);
		}

		/** Inserts an element into the ring buffer.
		 *
		 *  \warning Only one execution thread (main program thread or an ISR) may insert into a single buffer
		 *           otherwise data corruption may occur. Insertion and removal may occur from different execution
		 *           threads.
		 *
		 *  \param[in,out] Buffer  Pointer to a ring buffer structure to insert into.
		 *  \param[in]     Data    Data element to insert into the buffer.
		 */
		static inline void RingBuffer_Insert(RingBuffer_t* Buffer,
		                                     const u08 Data);
		static inline void RingBuffer_Insert(RingBuffer_t* Buffer,
		                                     const u08 Data)
		{
			*Buffer->In = Data;

			if (++Buffer->In == Buffer->End)
			  Buffer->In = Buffer->Start;

			u08	nointerrupted = 0;
			if (STATUS_REG & (1<<Interrupt_Flag))  // Если прерывания разрешены, то запрещаем их.
			{
				Disable_Interrupt
				nointerrupted = 1;					// И ставим флаг, что мы не в прерывании. 
			}

			Buffer->Count++;

			if (nointerrupted) Enable_Interrupt	
		}

		/** Removes an element from the ring buffer.
		 *
		 *  \warning Only one execution thread (main program thread or an ISR) may remove from a single buffer
		 *           otherwise data corruption may occur. Insertion and removal may occur from different execution
		 *           threads.
		 *
		 *  \param[in,out] Buffer  Pointer to a ring buffer structure to retrieve from.
		 *
		 *  \return Next data element stored in the buffer.
		 */
		static inline u08 RingBuffer_Remove(RingBuffer_t* Buffer);
		static inline u08 RingBuffer_Remove(RingBuffer_t* Buffer)
		{
			u08 Data = *Buffer->Out;

			if (++Buffer->Out == Buffer->End)
			  Buffer->Out = Buffer->Start;

			u08	nointerrupted = 0;
			if (STATUS_REG & (1<<Interrupt_Flag))  // Если прерывания разрешены, то запрещаем их.
			{
				Disable_Interrupt
				nointerrupted = 1;					// И ставим флаг, что мы не в прерывании. 
			}

			Buffer->Count--;

			if (nointerrupted) Enable_Interrupt	

			return Data;
		}

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

