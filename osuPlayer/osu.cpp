#include "pch.h"
#include "osu.h"

using namespace osu;
using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::FileProperties;

unsigned int ::osu::readULEB128FromReader(DataReader^ reader)
{
	unsigned int res = 0;
	unsigned char v;
	for (unsigned int i = 0; i < sizeof(res) * 8; i += 7) {
		v = reader->ReadByte();
		res |= ((v & 0x7f) << i);
		if (!(v & 0x80))
			return res;
	}
	throw ref new InvalidCastException();
}

String^ ::osu::readStringFromReader(DataReader^ reader)
{
	switch (reader->ReadByte()) {
	case 0x00:
		return nullptr;
	case 0x0b:
		break;
	default:
		throw ref new InvalidCastException();
	}
	unsigned int size = readULEB128FromReader(reader);
	return reader->ReadString(size);
}

void ::osu::readBeatmapFromReader(DataReader ^ reader, osuBeatmap * bmap)
{
	int32 size = reader->ReadInt32();
	auto rder = DataReader::FromBuffer(reader->ReadBuffer(size));

	bmap->artist = readStringFromReader(rder);
	readStringFromReader(rder);	// Artist in unicode
	bmap->title = readStringFromReader(rder);
	readStringFromReader(rder);	// Song title in unicode
	bmap->creator = readStringFromReader(rder);
	bmap->difficulty = readStringFromReader(rder);
}

IAsyncOperation<osuDB^>^ osuDB::loadFromFileAsync(StorageFile ^ file)
{
	return create_async([=]() -> osuDB^ {
		if (!file)
			return nullptr;

		osuDB^ db = ref new osuDB;
		uint32 size = 0;
		create_task(file->GetBasicPropertiesAsync()).then([&](BasicProperties^ prop) {
			size = (uint32)prop->Size;
		}).wait();
		if (!size)
			throw ref new InvalidArgumentException;

		DataReader^ reader;
		create_task(file->OpenSequentialReadAsync()).then([&](IInputStream^ in) {
			reader = ref new DataReader(in);
			reader->UnicodeEncoding = UnicodeEncoding::Utf8;
			reader->ByteOrder = ByteOrder::LittleEndian;
			return reader->LoadAsync(size);
		}).then([&](auto size) {
			// https://osu.ppy.sh/wiki/Db_(file_format)
			db->version = reader->ReadInt32();
			db->folderCount = reader->ReadInt32();
			db->unlocked = reader->ReadBoolean();
			reader->ReadInt64();	// Date the account will be unlocked
			db->playerName = readStringFromReader(reader);
			db->bmapCount = reader->ReadInt32();
			int count = db->bmapCount;
			while (count--) {
				osuBeatmap *bmap = new osuBeatmap;
				readBeatmapFromReader(reader, bmap);
				db->bmaps.push_back(bmap);
			}
		}).wait();
		return db;
	});
}

IAsyncOperation<::osu::osu^>^ ::osu::osu::loadFromFolderAsync(StorageFolder^ folder)
{
	return create_async([=] {
		auto o = ref new osu;
		create_task(folder->GetFileAsync("osu!.db")).then([&](task<StorageFile^> t) -> IAsyncOperation<osuDB^>^ {
			StorageFile^ file;
			try {
				file = t.get();
			} catch (Platform::COMException^ e) {
				//debug->Items->Append("Unable to open osu!.db from the specified directory: " + e->Message);
				o = nullptr;
			}
			if (o)
				return osuDB::loadFromFileAsync(file);
			else
				return nullptr;
		}).then([&](osuDB^ db) {
			o->_db = db;
		}).wait();
		return o;
	});
}
