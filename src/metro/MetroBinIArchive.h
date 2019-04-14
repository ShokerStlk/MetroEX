#pragma once

class MemStream;

class IMetroBinArchive
{
public:
    virtual bool IsBinArchive()        { return false; };
    virtual bool IsBinArrayArchive()   { return false; };

    virtual const    MemStream& GetRawStream() const     = 0;
    virtual          MemStream& GetRawDangerStream()     = 0; // use carefully
    virtual          MemStream  GetRawStreamCopy()       = 0;
    virtual          MemStream  GetRawStreamCopy() const = 0;
};
