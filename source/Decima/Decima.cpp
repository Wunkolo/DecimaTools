#include <Decima/Decima.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <cstring>

#include <immintrin.h>

// https://github.com/PeterScott/murmur3/blob/master/murmur3.c
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

static void MurmurHash3_x64_128 ( const void * key, const int len,
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

namespace Decima
{
	void Archive::FileHeader::Decrypt()
	{
		std::array<std::uint32_t,4> CurVec = MurmurSalt1;
		CurVec[0] = this->Key;
		std::array<std::uint32_t,4> CurHash;
		MurmurHash3_x64_128(
			CurVec.data(), 0x10, Decima::Archive::MurmurSeed, CurHash.data()
		);

		std::transform(
			CurVec.cbegin(), CurVec.cend(),
			CurHash.cbegin(),
			(std::uint32_t*)this + 2,
			std::bit_xor<std::uint32_t>()
		);

		CurVec = MurmurSalt1;
		CurVec[0] = this->Key + 1;
		MurmurHash3_x64_128(
			CurVec.data(), 0x10, Decima::Archive::MurmurSeed, CurHash.data()
		);

		std::transform(
			CurHash.cbegin(), CurHash.cend(),
			(const std::uint32_t*)this + 2 + 4,
			(std::uint32_t*)this + 2 + 4,
			std::bit_xor<std::uint32_t>()
		);
	}
	void Archive::FileEntry::Decrypt()
	{
		std::array<std::uint32_t,4> CurVec = MurmurSalt1;
		CurVec[0] = this->Unknown04;
		std::array<std::uint32_t,4> CurHash;
		MurmurHash3_x64_128(
			CurVec.data(), 0x10, Decima::Archive::MurmurSeed, CurHash.data()
		);

		std::transform(
			CurHash.cbegin(), CurHash.cend(),
			(const std::uint32_t*)this + 2,
			(std::uint32_t*)this,
			std::bit_xor<std::uint32_t>()
		);
		
		CurVec = MurmurSalt1;
		CurVec[0] = this->Span.Hash;
		MurmurHash3_x64_128(
			CurVec.data(), 0x10, Decima::Archive::MurmurSeed, CurHash.data()
		);

		std::transform(
			CurHash.cbegin(), CurHash.cend(),
			(const std::uint32_t*)this + 4,
			(std::uint32_t*)this + 4,
			std::bit_xor<std::uint32_t>()
		);
	}
	void Archive::ChunkEntry::Decrypt()
	{
		std::array<std::uint32_t,4> CurVec = MurmurSalt1;
		CurVec[0] = this->UncompressedSpan.Hash;
		std::array<std::uint32_t,4> CurHash;
		MurmurHash3_x64_128(
			CurVec.data(), 0x10, Decima::Archive::MurmurSeed, CurHash.data()
		);

		std::transform(
			CurHash.cbegin(), CurHash.cend(),
			(const std::uint32_t*)this,
			(std::uint32_t*)this,
			std::bit_xor<std::uint32_t>()
		);
		
		CurVec = MurmurSalt1;
		CurVec[0] = this->CompressedSpan.Hash;
		MurmurHash3_x64_128(
			CurVec.data(), 0x10, Decima::Archive::MurmurSeed, CurHash.data()
		);

		std::transform(
			CurHash.cbegin(), CurHash.cend(),
			(const std::uint32_t*)this + 4,
			(std::uint32_t*)this + 4,
			std::bit_xor<std::uint32_t>()
		);
	}

	Archive::Archive()
	{

	}

	Archive::~Archive()
	{

	}

	std::unique_ptr<Archive> Archive::OpenArchive(
		const std::filesystem::path& Path
	)
	{
		if( !std::filesystem::exists(Path) )			return nullptr;
		if( !std::filesystem::is_regular_file(Path) )	return nullptr;

		std::unique_ptr<Archive> NewArchive(new Archive());

		std::ptrdiff_t ReadPoint = 0;

		NewArchive->FileMapping = mio::ummap_source(Path.c_str());
		NewArchive->Header = *reinterpret_cast<const Archive::FileHeader*>(
			NewArchive->FileMapping.data()
		);
		ReadPoint += sizeof(Archive::FileHeader);

		switch( NewArchive->Header.Version )
		{
			case ArchiveVersion::Unencrypted:
			{
				break;
			}
			case ArchiveVersion::Encrypted:
			{
				NewArchive->Header.Decrypt();
				break;
			}
			// Not a valid archive file
			default: return nullptr;
		}

		// Load file entries
		NewArchive->FileEntries.resize(NewArchive->Header.FileTableCount);
		std::memcpy(
			NewArchive->FileEntries.data(),
			NewArchive->FileMapping.data() + ReadPoint,
			sizeof(Decima::Archive::FileEntry) * NewArchive->Header.FileTableCount
		);
		if(	NewArchive->Encrypted() )
		{
			for(auto& CurEntry : NewArchive->FileEntries) CurEntry.Decrypt();
		}
		ReadPoint += sizeof(Decima::Archive::FileEntry) * NewArchive->Header.FileTableCount;

		// Load chunk entries
		NewArchive->ChunkEntries.resize(NewArchive->Header.ChunkTableCount);
		std::memcpy(
			NewArchive->ChunkEntries.data(),
			NewArchive->FileMapping.data() + ReadPoint,
			sizeof(Decima::Archive::ChunkEntry) * NewArchive->Header.ChunkTableCount
		);
		if(	NewArchive->Encrypted() )
		{
			for(auto& CurChunk : NewArchive->ChunkEntries) CurChunk.Decrypt();
		}
		ReadPoint += sizeof(Decima::Archive::ChunkEntry) * NewArchive->Header.ChunkTableCount;

		return NewArchive;
	}
}
