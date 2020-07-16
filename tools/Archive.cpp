#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vector>

#include <Decima/Decima.hpp>
#include <mio/mmap.hpp>

#include <immintrin.h>

// https://github.com/PeterScott/murmur3/blob/master/murmur3.c
void MurmurHash3_x64_128 ( const void * key, const int len,
						   const uint32_t seed, void * out );

int main(int argc, char* argv[])
{
	if( argc < 2 )
	{
		return EXIT_FAILURE;
	}

	mio::ummap_source FileMapping(argv[1]);

	Decima::FileHeader Header;

	Header = *reinterpret_cast<const Decima::FileHeader*>(FileMapping.data());

	const __m128i MurmurSalt = _mm_loadu_si128((const __m128i*)Decima::MurmurSalt.data());

	{
		__m128i CurHashInput;
		__m128i CurHash;
		__m128i CurVec = _mm_blend_epi16(
			MurmurSalt,
			_mm_set1_epi32(((std::uint32_t*)&Header)[1]),
			3
		);
		CurHashInput = CurVec;
		MurmurHash3_x64_128(&CurHashInput, 0x10, Decima::MurmurSeed, &CurHash);
		CurVec = _mm_xor_si128(
			_mm_loadu_si128((__m128i*)&(((std::uint32_t*)&Header)[2])),
			CurHash
		);
		_mm_storeu_si128((__m128i*)&(((std::uint32_t*)&Header)[2]), CurVec);


		CurVec = _mm_blend_epi16(
			MurmurSalt,
			_mm_set1_epi32(Header.Version + 1),
			3
		);
		CurHashInput = CurVec;
		MurmurHash3_x64_128(&CurHashInput, 0x10, Decima::MurmurSeed, &CurHash);
		CurVec = _mm_xor_si128(
			_mm_loadu_si128((__m128i*)&(((std::uint32_t*)&Header)[2 + 4])),
			CurHash
		);
		_mm_storeu_si128((__m128i*)&(((std::uint32_t*)&Header)[2 + 4]), CurVec);
	}

	std::printf(
		"Magic:               %12.08X\n"
		"Version:             %12.08X\n"
		"FileSize:            %12lu\n"
		"DataSize:            %12lu\n"
		"FileTableCount:      %12lu\n"
		"ChunkTableCount:     %12u\n"
		"MaxChunkSize:        %12u\n",
		Header.Magic,
		Header.Version,
		Header.FileSize,
		Header.DataSize,
		Header.FileTableCount,
		Header.ChunkTableCount,
		Header.MaxChunkSize
	);

	std::vector<Decima::FileEntry> FileEntries;
	FileEntries.resize(Header.FileTableCount);
	std::memcpy(
		FileEntries.data(),
		FileMapping.data() + sizeof(Decima::FileHeader),
		sizeof(Decima::FileEntry) * Header.FileTableCount
	);

	for(Decima::FileEntry& CurEntry : FileEntries)
	{
		{
			__m128i CurHashInput;
			__m128i CurHash;
			__m128i CurVec = _mm_blend_epi16(
				MurmurSalt,
				_mm_set1_epi32(((std::uint32_t*)&CurEntry)[1]),
				3
			);
			CurHashInput = CurVec;
			MurmurHash3_x64_128(&CurHashInput, 0x10, Decima::MurmurSeed, &CurHash);

			CurVec = _mm_xor_si128(
				_mm_loadu_si128((__m128i*)&(((std::uint32_t*)&CurEntry)[2])),
				CurHash
			);
			_mm_storeu_si128((__m128i*)&CurEntry, CurVec);


			CurVec = _mm_blend_epi16(
				MurmurSalt,
				_mm_set1_epi32(CurEntry.Unknown1C),
				3
			);
			CurHashInput = CurVec;
			MurmurHash3_x64_128(&CurHashInput, 0x10, Decima::MurmurSeed, &CurHash);
			CurVec = _mm_xor_si128(
				_mm_loadu_si128((__m128i*)&(((std::uint32_t*)&CurEntry)[4])),
				CurHash
			);
			_mm_storeu_si128((__m128i*)&(((std::uint32_t*)&CurEntry)[4]), CurVec);
		}
		std::printf(
			"\tEntryID:       %24.u\n"
			"\t- Unknown04:   %24.u\n"
			"\t- Unknown08:   %24.016lX\n"
			"\t- Offset:      %24lu\n"
			"\t- Size:        %24u\n"
			"\t- Unknown1C:   %24.08X\n",
			CurEntry.EntryID,
			CurEntry.Unknown04,
			CurEntry.Unknown08,
			CurEntry.Offset,
			CurEntry.Size,
			CurEntry.Unknown1C
		);
	}
	
	return 0;
}


inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
  return (x << r) | (x >> (32 - r));
}

inline uint64_t rotl64 ( uint64_t x, int8_t r )
{
  return (x << r) | (x >> (64 - r));
}

inline uint64_t fmix64 ( uint64_t k )
{
  k ^= k >> 33;
  k *= 0xff51afd7ed558ccdULL;
  k ^= k >> 33;
  k *= 0xc4ceb9fe1a85ec53ULL;
  k ^= k >> 33;

  return k;
}

void MurmurHash3_x64_128 ( const void * key, const int len,
						   const uint32_t seed, void * out )
{
	const uint8_t * data = (const uint8_t*)key;
	const int nblocks = len / 16;
	int i;

	uint64_t h1 = seed;
	uint64_t h2 = seed;

	uint64_t c1 = 0x87c37b91114253d5LLU;
	uint64_t c2 = 0x4cf5ad432745937fLLU;

	//----------
	// body

	const uint64_t * blocks = (const uint64_t *)(data);

	for(i = 0; i < nblocks; i++)
	{
	uint64_t k1 = blocks[i*2+0];
	uint64_t k2 = blocks[i*2+1];

	k1 *= c1; k1  = rotl64(k1,31); k1 *= c2; h1 ^= k1;

	h1 = rotl64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;

	k2 *= c2; k2  = rotl64(k2,33); k2 *= c1; h2 ^= k2;

	h2 = rotl64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
	}

	//----------
	// tail

	const uint8_t * tail = (const uint8_t*)(data + nblocks*16);

	uint64_t k1 = 0;
	uint64_t k2 = 0;

	switch(len & 15)
	{
	case 15: k2 ^= (uint64_t)(tail[14]) << 48;   [[fallthrough]];
	case 14: k2 ^= (uint64_t)(tail[13]) << 40;   [[fallthrough]];
	case 13: k2 ^= (uint64_t)(tail[12]) << 32;   [[fallthrough]];
	case 12: k2 ^= (uint64_t)(tail[11]) << 24;   [[fallthrough]];
	case 11: k2 ^= (uint64_t)(tail[10]) << 16;   [[fallthrough]];
	case 10: k2 ^= (uint64_t)(tail[ 9]) << 8;    [[fallthrough]];
	case  9: k2 ^= (uint64_t)(tail[ 8]) << 0;
			k2 *= c2; k2  = rotl64(k2,33); k2 *= c1; h2 ^= k2;
			[[fallthrough]];

	case  8: k1 ^= (uint64_t)(tail[ 7]) << 56;    [[fallthrough]];
	case  7: k1 ^= (uint64_t)(tail[ 6]) << 48;    [[fallthrough]];
	case  6: k1 ^= (uint64_t)(tail[ 5]) << 40;    [[fallthrough]];
	case  5: k1 ^= (uint64_t)(tail[ 4]) << 32;    [[fallthrough]];
	case  4: k1 ^= (uint64_t)(tail[ 3]) << 24;    [[fallthrough]];
	case  3: k1 ^= (uint64_t)(tail[ 2]) << 16;    [[fallthrough]];
	case  2: k1 ^= (uint64_t)(tail[ 1]) << 8;     [[fallthrough]];
	case  1: k1 ^= (uint64_t)(tail[ 0]) << 0;     
			k1 *= c1; k1  = rotl64(k1,31); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization

	h1 ^= len; h2 ^= len;

	h1 += h2;
	h2 += h1;

	h1 = fmix64(h1);
	h2 = fmix64(h2);

	h1 += h2;
	h2 += h1;

	((uint64_t*)out)[0] = h1;
	((uint64_t*)out)[1] = h2;
}