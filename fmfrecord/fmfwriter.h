#ifndef FMFWRITER_H
#define FMFWRITER_H

class FmfWriter
{
	private:
		FILE *fp;
		FILE *flog;

		char fname[100];
		char flogname[100];

		unsigned __int32 fmfVersion, SizeY, SizeX;
		unsigned __int64 bytesPerChunk;
		char *buf;

	public:
		int id;
		unsigned __int64 nframes;

		FmfWriter();

		int Open();
		int Close();

		void InitHeader(unsigned __int32 x, unsigned __int32 y);
		void WriteHeader();
		void WriteFrame(Spinnaker::ImagePtr img);
		void WriteLog(Spinnaker::ImagePtr img);  //Spinnaker::ChunkData chunky);
		//void WriteFrame(FlyCapture2::image img);
		//void WriteLog(FlyCapture2::timestamp st);
		int IsOpen();
};

#endif