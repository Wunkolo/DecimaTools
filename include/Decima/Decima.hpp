#include <cstdint>
#include <cstddef>
#include <array>

#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <ostream>

#include <mio/mmap.hpp>

namespace Decima
{
class Archive
{
public:
	~Archive();
	// Used for file headers, file table entries, Segment table entires
	static constexpr std::array<std::uint32_t, 4> MurmurSalt1 = {
		0x0FA3A9443, 0x0F41CAB62, 0x0F376811C, 0x0D2A89E3E
	};

	// Used for file data Segments
	static constexpr std::array<std::uint32_t, 4> MurmurSalt2 = {
		0x06C084A37, 0x07E159D95, 0x03D5AF7E8, 0x018AA7D3F
	};

	static constexpr std::uint32_t MurmurSeed = 42u;

	enum class ArchiveVersion : std::uint32_t
	{
		// Horizon Zero Dawn(PS4)
		Unencrypted		= 0x20304050,
		// Death Stranding(PC)
		Encrypted		= 0x21304050,
	};

	#pragma pack(push,1)
	struct FileSpan
	{
		std::uint64_t Offset;
		std::uint32_t Size;
		// Used during Decryption, likely a checksum
		std::uint32_t Hash;
	};
	// Thanks Jayveer for actually being open about these structs
	// https://github.com/Jayveer/Decima-Explorer/blob/master/archive/DecimaArchive.h
	struct FileHeader
	{
		ArchiveVersion	Version;
		std::uint32_t	Key;
		std::uint64_t	FileSize;
		std::uint64_t	DataSize;
		std::uint64_t	FileTableCount;
		std::uint32_t	SegmentTableCount;
		std::uint32_t	MaxSegmentSize;

		void Decrypt();
	};
	static_assert(sizeof(FileHeader) == 0x28);

	struct FileEntry
	{
		std::uint32_t	FileID;
		std::uint32_t	Unknown04;
		std::uint64_t	Unknown08;
		FileSpan		Span;
		void Decrypt();
	};
	static_assert(sizeof(FileEntry) == 0x20);

	struct SegmentEntry
	{
		FileSpan	UncompressedSpan;
		FileSpan	CompressedSpan;
		void Decrypt();
	};
	static_assert(sizeof(SegmentEntry) == 0x20);
	#pragma pack(pop)


	inline bool Encrypted() const
	{
		return Header.Version == ArchiveVersion::Encrypted;
	}

	inline const FileHeader& GetHeader() const
	{
		return Header;
	}

	inline void IterateFileEntries(
		std::function<void(const FileEntry&)> Proc
	) const 
	{
		std::for_each(FileEntries.cbegin(), FileEntries.cend(), Proc);
	}

	inline void IterateSegmentEntries(
		std::function<void(const SegmentEntry&)> Proc
	) const 
	{
		std::for_each(SegmentEntries.cbegin(), SegmentEntries.cend(), Proc);
	}

	std::optional<std::reference_wrapper<const Archive::FileEntry>> GetFileEntry(
		std::uint32_t FileID
	) const;

	// Given an offset, find out what segment we land in
	std::optional<std::reference_wrapper<const Archive::SegmentEntry>> GetSegmentCompressed(
		std::uint64_t Offset
	) const;
	std::optional<std::reference_wrapper<const Archive::SegmentEntry>> GetSegmentUncompressed(
		std::uint64_t Offset
	) const;

	bool ExtractFile(const FileEntry& FileEntry, std::ostream& OutStream) const;

	static std::unique_ptr<Archive> OpenArchive(
		const std::filesystem::path& Path
	);
private:
	Archive();
	FileHeader Header;
	//  Used to easily lookup an EntryID into an index into FileEntries
	std::unordered_map<std::uint32_t, std::size_t> FileEntryLut;
	std::vector<FileEntry> FileEntries;

	// Acceleration structure to map an offset into a Segment entry
	std::map<std::uint64_t,const SegmentEntry&> SegmentCompressedLut;
	std::map<std::uint64_t,const SegmentEntry&> SegmentUncompressedLut;
	std::vector<SegmentEntry> SegmentEntries;

	mio::ummap_source FileMapping;
};
}
