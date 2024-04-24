#ifndef QTE_H
#define QTE_H

#pragma once
class QuadTreeEncoder : public Encoder {
public:

    explicit QuadTreeEncoder(bool isTextOutput, const std::string& outputFile,int quality = 100);

    ~QuadTreeEncoder() override;

    Transforms *Encode(Image *source) override;

private:
    void findMatchesFor(Transform &transforms, int toX, int toY, int blockSize);

private:
    int quality;

};

#endif // QTE_H
