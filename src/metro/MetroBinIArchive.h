#pragma once

class MemStream;

class IMetroBinArchive
{
public:
    virtual bool IsBinArchive()        { return false; };
    virtual bool IsBinArrayArchive()   { return false; };

    inline virtual const    CharString& GetFileName() const     = 0;

    inline virtual const    MemStream& GetRawStream() const     = 0;
    inline virtual          MemStream& GetRawDangerStream()     = 0; // use carefully
    inline virtual          MemStream  GetRawStreamCopy()       = 0;
    inline virtual          MemStream  GetRawStreamCopy() const = 0;
};
