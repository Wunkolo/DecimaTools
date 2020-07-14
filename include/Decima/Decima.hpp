#include <cstdint>
#include <cstddef>

namespace Decima
{
	#pragma pack(push,1)

	struct FileHeader
	{
		std::uint32_t Magic;
		std::uint32_t Version;
		std::uint64_t FileSize;
		std::uint64_t DataSize;
		std::uint64_t FileTableCount;
		std::uint32_t ChunkTableCount;
		std::uint32_t MaxChunkSize;
	};

	struct FileEntry
	{
		std::uint32_t EntryID;
		std::uint32_t Unknown04;
		std::uint64_t Unknown08;
		std::uint64_t Offset;
		std::uint32_t Size;
		std::uint32_t Unknown1C;
	};

	struct ChunkEntry
	{
		std::uint64_t OffsetUncompressed;
		std::uint32_t SizeUncompresed;
		std::uint32_t Unknown0C;
		std::uint64_t OffsetCompressed;
		std::uint32_t SizeCompressed;
		std::uint32_t Unknown1C;
	};
	#pragma pack(pop)
}