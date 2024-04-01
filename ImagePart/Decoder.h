#ifndef DEC_H
#define DEC_H

#pragma once
class Decoder {
public:

    Decoder(int width, int height, int channels);

    ~Decoder();

    void Decode(Transforms *transforms);

    Image *GetNewImage(const std::string &fileName, int channel) const;

private:
    Image img;
};

#endif // DEC_H


