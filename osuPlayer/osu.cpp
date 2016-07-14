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
	throw ref new InvalidCastException;
}

String^ ::osu::readStringFromReader(DataReader^ reader)
{
	switch (reader->ReadByte()) {
	case 0x00:
		return nullptr;
	case 0x0b:
		break;
	default:
		throw ref new InvalidCastException;
	}
	unsigned int size = readULEB128FromReader(reader);
	return reader->ReadString(size);
}

void ::osu::readIDPairsFromReader(DataReader ^ reader, vector<idPair>* pairs)
{
	int32 count = reader->ReadInt32();
	while (count--) {
		idPair pair;
		if (reader->ReadByte() != 0x08)
			throw ref new InvalidCastException;
		pair.i = reader->ReadInt32();
		if (reader->ReadByte() != 0x0d)
			throw ref new InvalidCastException;
		pair.d = reader->ReadDouble();
		pairs->push_back(pair);
	}
}

void ::osu::readTimingPointsFromReader(DataReader ^ reader, vector<timingPoint>* tpoints)
{
	int32 count = reader->ReadInt32();
	while (count--) {
		timingPoint tpoint = { reader->ReadDouble(), reader->ReadDouble(), reader->ReadBoolean() };
		tpoints->push_back(tpoint);
	}
}

void ::osu::readBeatmapFromReader(DataReader ^ reader, osuBeatmap * bmap, int32 version)
{
	int32 size = reader->ReadInt32();
	auto rder = DataReader::FromBuffer(reader->ReadBuffer(size));
	rder->ByteOrder = ByteOrder::LittleEndian;

	reader->UnicodeEncoding = UnicodeEncoding::Utf8;
	bmap->artist = readStringFromReader(rder);
	reader->UnicodeEncoding = UnicodeEncoding::Utf16LE;
	bmap->artistUnicode = readStringFromReader(rder);
	reader->UnicodeEncoding = UnicodeEncoding::Utf8;
	bmap->title = readStringFromReader(rder);
	reader->UnicodeEncoding = UnicodeEncoding::Utf16LE;
	bmap->titleUnicode = readStringFromReader(rder);
	reader->UnicodeEncoding = UnicodeEncoding::Utf8;
	bmap->creator = readStringFromReader(rder);
	bmap->difficulty = readStringFromReader(rder);
	bmap->audioFile = readStringFromReader(rder);
	bmap->hash = readStringFromReader(rder);
	bmap->osuFile = readStringFromReader(rder);
	bmap->ranked = rder->ReadByte();
	bmap->hitcircles = rder->ReadInt16();
	bmap->sliders = rder->ReadInt16();
	bmap->spinners = rder->ReadInt16();
	bmap->lastModTime = rder->ReadInt64();
	if (version < 20140609) {
		bmap->approachRate = rder->ReadByte();
		bmap->circleSize = rder->ReadByte();
		bmap->hpDrain = rder->ReadByte();
		bmap->overallDifficulty = rder->ReadByte();
	} else {
		bmap->approachRate = rder->ReadSingle();
		bmap->circleSize = rder->ReadSingle();
		bmap->hpDrain = rder->ReadSingle();
		bmap->overallDifficulty = rder->ReadSingle();
	}
	bmap->sliderVelocity = rder->ReadDouble();
	if (version >= 20140609)
		for (int i = 0; i != 4; i++)
			readIDPairsFromReader(rder, &bmap->ratings[i]);
	bmap->drainTime = rder->ReadInt32();
	bmap->totalTime = rder->ReadInt32();
	bmap->previewTime = rder->ReadInt32();
	readTimingPointsFromReader(rder, &bmap->tPoints);
	bmap->bmapID = rder->ReadInt32();
	bmap->bmapSetID = rder->ReadInt32();
	bmap->threadID = rder->ReadInt32();
	for (int i = 0; i != 4; i++)
		bmap->grade[i] = rder->ReadByte();
	bmap->localOffset = rder->ReadInt16();
	bmap->leniency = rder->ReadSingle();
	bmap->mode = rder->ReadByte();
	bmap->source = readStringFromReader(rder);
	bmap->tags = readStringFromReader(rder);
	bmap->onlineOffset = rder->ReadInt16();
	bmap->titleFont = readStringFromReader(rder);
	bmap->unplayed = rder->ReadBoolean();
	bmap->lastPlayTime = rder->ReadInt64();
	bmap->osz2 = rder->ReadBoolean();
	bmap->folder = readStringFromReader(rder);
	bmap->lastChecked = rder->ReadInt64();
	bmap->ignoreSounds = rder->ReadBoolean();
	bmap->ignoreSkin = rder->ReadBoolean();
	bmap->disableSB = rder->ReadBoolean();
	bmap->disableVideo = rder->ReadBoolean();
	bmap->visualOverride = rder->ReadBoolean();
	if (version < 20140609)
		bmap->unknown = rder->ReadInt16();
	bmap->modTime = rder->ReadInt32();
	bmap->maniaSpeed = rder->ReadByte();
}

IAsyncOperation<bool>^ ::osu::loadDBFromFileAsync(StorageFile^ file, osuDB *db)
{
	return create_async([=]() {
		if (!file || !db)
			return false;

		uint32 size = 0;
		create_task(file->GetBasicPropertiesAsync()).then([&](BasicProperties^ prop) {
			size = (uint32)prop->Size;
		}).wait();
		if (!size)
			return false;
			//throw ref new InvalidArgumentException;

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
				readBeatmapFromReader(reader, bmap, db->version);
				db->bmaps.push_back(bmap);
			}
		}).wait();
		return true;
	});
}

IAsyncOperation<bool>^ ::osu::loadFromFolderAsync(StorageFolder^ folder, ::osu::osu *o)
{
	return create_async([=] {
		if (!folder || !o)
			return false;

		o->valid = false;
		o->dir = folder;
		create_task(folder->GetFileAsync("osu!.db")).then([&](task<StorageFile^> t) {
			StorageFile^ file = nullptr;
			try {
				file = t.get();
			} catch (COMException^ e) {}
			o->fileDB = file;
			return folder->GetFolderAsync("Songs");
		}).then([=](task<StorageFolder^> t) -> IAsyncOperation<bool>^ {
			StorageFolder^ folder;
			try {
				folder = t.get();
			} catch (COMException^ e) {
				return create_async([] {return false; });
			}
			o->dirSongs = folder;
			return loadDBFromFileAsync(o->fileDB, &o->db);
		}).then([&](bool res) {
			o->valid = res;
		}).wait();
		return o->valid;
	});
}
