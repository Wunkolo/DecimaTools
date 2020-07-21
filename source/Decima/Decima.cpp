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
	case 10: k2 ^= (uint64_t)(tail[ 9]) << 8;	[[fallthrough]];
	case  9: k2 ^= (uint64_t)(tail[ 8]) << 0;
			k2 *= c2; k2  = rotl64(k2,33); k2 *= c1; h2 ^= k2;
			[[fallthrough]];

	case  8: k1 ^= (uint64_t)(tail[ 7]) << 56;	[[fallthrough]];
	case  7: k1 ^= (uint64_t)(tail[ 6]) << 48;	[[fallthrough]];
	case  6: k1 ^= (uint64_t)(tail[ 5]) << 40;	[[fallthrough]];
	case  5: k1 ^= (uint64_t)(tail[ 4]) << 32;	[[fallthrough]];
	case  4: k1 ^= (uint64_t)(tail[ 3]) << 24;	[[fallthrough]];
	case  3: k1 ^= (uint64_t)(tail[ 2]) << 16;	[[fallthrough]];
	case  2: k1 ^= (uint64_t)(tail[ 1]) << 8;	 [[fallthrough]];
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

// https://github.com/pod32g/MD5/blob/master/md5.c
void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest)
{

	static constexpr uint32_t k[64] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};

	// r specifies the per-round shift amounts
	static constexpr uint32_t r[64] = {
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
	};

	// leftrotate function definition
	#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

	// These vars will contain the hash
	uint32_t h0, h1, h2, h3;

	// Message (to prepare)
	uint8_t *msg = NULL;

	size_t new_len, offset;
	uint32_t w[16];
	uint32_t a, b, c, d, i, f, g, temp;

	// Initialize variables - simple count in nibbles:
	h0 = 0x67452301;
	h1 = 0xefcdab89;
	h2 = 0x98badcfe;
	h3 = 0x10325476;

	//Pre-processing:
	//append "1" bit to message
	//append "0" bits until message length in bits ≡ 448 (mod 512)
	//append length mod (2^64) to message

	for (new_len = initial_len + 1; new_len % (512/8) != 448/8; new_len++);

	msg = new uint8_t[new_len + 8]();
	memcpy(msg, initial_msg, initial_len);
	msg[initial_len] = 0x80; // append the "1" bit; most significant bit is "first"
	for (offset = initial_len + 1; offset < new_len; offset++)
		msg[offset] = 0; // append "0" bits

	// append the len in bits at the end of the buffer.
	*((std::uint32_t*)(msg + new_len)) = initial_len*8;
	// initial_len>>29 == initial_len*8>>32, but avoids overflow.
	*((std::uint32_t*)(msg + new_len + 4)) = initial_len>>29;

	// Process the message in successive 512-bit Segments:
	//for each 512-bit Segment of message:
	for(offset=0; offset<new_len; offset += (512/8)) {

		// break Segment into sixteen 32-bit words w[j], 0 ≤ j ≤ 15
		for (i = 0; i < 16; i++)
			w[i] = ((std::uint32_t*)(msg + offset))[i];

		// Initialize hash value for this Segment:
		a = h0;
		b = h1;
		c = h2;
		d = h3;

		// Main loop:
		for(i = 0; i<64; i++) {

			if (i < 16) {
				f = (b & c) | ((~b) & d);
				g = i;
			} else if (i < 32) {
				f = (d & b) | ((~d) & c);
				g = (5*i + 1) % 16;
			} else if (i < 48) {
				f = b ^ c ^ d;
				g = (3*i + 5) % 16;
			} else {
				f = c ^ (b | (~d));
				g = (7*i) % 16;
			}

			temp = d;
			d = c;
			c = b;
			b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
			a = temp;

		}

		// Add this Segment's hash to result so far:
		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;

	}
	delete[] msg;

	((std::uint32_t*)digest)[0] = h0;
	((std::uint32_t*)digest)[1] = h1;
	((std::uint32_t*)digest)[2] = h2;
	((std::uint32_t*)digest)[3] = h3;
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
	void Archive::SegmentEntry::Decrypt()
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

	std::optional<std::reference_wrapper<const Archive::FileEntry>> Archive::GetFileEntry(
		std::uint32_t FileID
	) const
	{
		if( FileEntryLut.count(FileID) )
			return FileEntries.at(FileEntryLut.at(FileID));
		return std::nullopt;
	}

	std::optional<std::reference_wrapper<const Archive::SegmentEntry>> Archive::GetSegmentCompressed(
		std::uint64_t Offset
	) const
	{
		if( const auto LowerBound = SegmentCompressedLut.lower_bound(Offset); LowerBound != SegmentCompressedLut.cend())
		{
			const SegmentEntry& Segment = LowerBound->second;
			// Must be within the chunk's span
			if( Offset - Segment.CompressedSpan.Offset < Segment.CompressedSpan.Size)
			{
				return Segment;
			}
		}
		return std::nullopt;
	}

	std::optional<std::reference_wrapper<const Archive::SegmentEntry>> Archive::GetSegmentUncompressed(
		std::uint64_t Offset
	) const
	{
		if( const auto LowerBound = SegmentUncompressedLut.lower_bound(Offset); LowerBound != SegmentUncompressedLut.cend())
		{
			const SegmentEntry& Segment = LowerBound->second;
			// Must be within the chunk's span
			if( Offset - Segment.UncompressedSpan.Offset < Segment.UncompressedSpan.Size)
			{
				return Segment;
			}
		}
		return std::nullopt;
	}

	bool Archive::ExtractFile(const FileEntry& FileEntry, std::ostream& OutStream) const
	{
		if(!OutStream) return false;

		// Read buffer
		std::vector<std::byte> SegmentData;
		SegmentData.resize(Header.MaxSegmentSize);

		std::size_t ToRead = FileEntry.Span.Size;

		SegmentEntry CurSegment;
		while( ToRead )
		{
			if( auto FindSegment = GetSegmentUncompressed(FileEntry.Span.Offset + (FileEntry.Span.Size - ToRead)); FindSegment)
			{
				CurSegment = FindSegment.value();
			}
			else
			{
				// Error mapping offset
				return false;
			}
			// Read Compressed Segment data
			std::memcpy(
				SegmentData.data(),
				FileMapping.data() + CurSegment.CompressedSpan.Offset,
				CurSegment.CompressedSpan.Size
			);
			// Decrypt Segment
			if( Encrypted() )
			{
				std::array<std::uint32_t, 4> Key;
				// Hash first 16 bytes of Segment entry
				MurmurHash3_x64_128(&CurSegment, 0x10, MurmurSeed, Key.data());
				// Xor it
				std::transform(
					MurmurSalt2.cbegin(), MurmurSalt2.cend(),
					Key.cbegin(), Key.begin(),
					std::bit_xor<std::uint32_t>()
				);
				// MD5 hash it
				md5((const std::uint8_t*)Key.data(), 0x10, (std::uint8_t*)Key.data());

				// Xor the chunk data with this hash, modulo 16
				for( std::size_t i = 0; i < CurSegment.CompressedSpan.Size; ++i)
				{
					SegmentData[i] ^= reinterpret_cast<const std::byte*>(Key.data())[i % 16];
				}
			}
			
			// Decompress Segment

			// Write decompressed data to file
			OutStream.write(
				reinterpret_cast<const char*>(SegmentData.data()),
				CurSegment.UncompressedSpan.Size
			);
			ToRead -= CurSegment.UncompressedSpan.Size;
		}
		return true;
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
			for( auto& CurEntry : NewArchive->FileEntries ) CurEntry.Decrypt();
		}
		ReadPoint += sizeof(Decima::Archive::FileEntry) * NewArchive->Header.FileTableCount;

		// Generate file-entry LUT
		for( std::size_t i = 0; i < NewArchive->FileEntries.size(); ++i)
		{
			NewArchive->FileEntryLut.emplace(NewArchive->FileEntries[i].FileID, i);
		}

		// Load Segment entries
		NewArchive->SegmentEntries.resize(NewArchive->Header.SegmentTableCount);
		std::memcpy(
			NewArchive->SegmentEntries.data(),
			NewArchive->FileMapping.data() + ReadPoint,
			sizeof(Decima::Archive::SegmentEntry) * NewArchive->Header.SegmentTableCount
		);
		if(	NewArchive->Encrypted() )
		{
			for( auto& CurSegment : NewArchive->SegmentEntries ) CurSegment.Decrypt();
		}
		// Build Segment LUTs
		for(const auto& CurSegment : NewArchive->SegmentEntries)
		{
			NewArchive->SegmentCompressedLut.emplace(CurSegment.CompressedSpan.Offset, CurSegment);
			NewArchive->SegmentUncompressedLut.emplace(CurSegment.UncompressedSpan.Offset, CurSegment);
		}
		ReadPoint += sizeof(Decima::Archive::SegmentEntry) * NewArchive->Header.SegmentTableCount;

		return NewArchive;
	}
}
