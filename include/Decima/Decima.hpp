#include <cstdint>
#include <cstddef>
#include <array>

namespace Decima
{
	constexpr std::array<std::uint32_t, 4> MurmurSalt = {
		0x0FA3A9443,
		0x0F41CAB62,
		0x0F376811C,
		0x0D2A89E3E
	};

	constexpr std::uint32_t MurmurSeed = 42u;
	#pragma pack(push,1)

	// Thanks Jayveer for actually being open about these structs
	// https://github.com/Jayveer/Decima-Explorer/blob/master/archive/DecimaArchive.h
	struct FileHeader
	{
		// 0x20304050 : Horizon Zero Dawn(PS4)
		// 0x21304050 : Death Stranding(PC)
		std::uint32_t Magic;
		std::uint32_t Version;
		std::uint64_t FilebSize;
		std::uint64_t DataSize;
		std::uint64_t FileTableCount;
		std::uint32_t ChunkTableCount;
		std::uint32_t MaxChunkSize;
	};
	static_assert(sizeof(FileHeader) == 0x28);

	struct FileEntry
	{
		std::uint32_t EntryID;
		std::uint32_t Unknown04;
		std::uint64_t Unknown08;
		std::uint64_t Offset;
		std::uint32_t Size;
		std::uint32_t Unknown1C;
	};
	static_assert(sizeof(FileEntry) == 0x20);

	struct ChunkEntry
	{
		std::uint64_t OffsetUncompressed;
		std::uint32_t SizeUncompresed;
		std::uint32_t Unknown0C;
		std::uint64_t OffsetCompressed;
		std::uint32_t SizeCompressed;
		std::uint32_t Unknown1C;
	};
	static_assert(sizeof(ChunkEntry) == 0x20);
	#pragma pack(pop)
}
