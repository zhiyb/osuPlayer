#pragma once

namespace osu
{
	using namespace concurrency;
	using namespace Platform;
	using namespace Platform::Collections;
	using namespace Windows::Foundation;
	using namespace Windows::Storage;
	using namespace Windows::Storage::Streams;

	using std::vector;

	struct idPair
	{
		int i;
		double d;
	};

	struct timingPoint
	{
		double bpm;
		double offset;		// ms
		bool nonInherited;
	};

	struct osuBeatmap
	{
		String^ getArtist() { return artistUnicode ? artistUnicode : artist; }
		String^ getTitle() { return titleUnicode ? titleUnicode : title; }

		String ^artist, ^artistUnicode, ^title, ^titleUnicode, ^creator;
		String ^difficulty;
		String ^audioFile, ^hash, ^osuFile;
		int dummy;
		int8 ranked;
		int16 hitcircles, sliders, spinners;
		int64 lastModTime;
		float approachRate, circleSize, hpDrain, overallDifficulty;
		double sliderVelocity;
		vector<idPair> ratings[4];
		int32 drainTime;	// s
		int32 totalTime;	// ms
		int32 previewTime;	// ms
		vector<timingPoint> tPoints;
		int32 bmapID, bmapSetID, threadID;
		int8 grade[4];
		int16 localOffset;
		float leniency;
		int8 mode;
		String ^source, ^tags;
		int16 onlineOffset;
		String ^titleFont;
		bool unplayed;
		int64 lastPlayTime;
		bool osz2;
		String ^folder;
		int64 lastChecked;
		bool ignoreSounds, ignoreSkin;
		bool disableSB, disableVideo, visualOverride;
		int16 unknown;
		int32 modTime;
		char maniaSpeed;
	};

	struct osuDB
	{
		~osuDB() { for (auto bmap : bmaps) delete bmap; }

		int32 version;
		int32 folderCount;
		bool unlocked;
		String^ playerName;
		int32 bmapCount;
		vector<osuBeatmap *> bmaps;
	};

	struct osu
	{
		bool valid;
		osuDB db;

		StorageFile ^fileDB;
		StorageFolder ^dir;
		StorageFolder ^dirSongs;
	};

	unsigned int readULEB128FromReader(DataReader^ reader);
	String^ readStringFromReader(DataReader^ reader);
	void readIDPairsFromReader(DataReader^ reader, vector<idPair> *pairs);
	void readTimingPointsFromReader(DataReader^ reader, vector<timingPoint> *tpoints);
	void readBeatmapFromReader(DataReader^ reader, osuBeatmap *bmap, int32 version);
	IAsyncOperation<bool>^ loadDBFromFileAsync(StorageFile^ file, osuDB *db);
	IAsyncOperation<bool>^ loadFromFolderAsync(StorageFolder^ folder, osu *o);
}
