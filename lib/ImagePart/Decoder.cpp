#include <string>
#include <vector>
#include "Image.h"
#include "IFSTransform.h"
#include "Decoder.h"

void Decoder::Decode(Transforms *transforms) {
    Transform::iterator iter;

    img.channels = transforms->channels;

    for (int channel = 1; channel <= img.channels; channel++) {
        PixelValue *origImage = img.imagedata;
        if (channel == 2)
            origImage = img.imagedata2;
        else if (channel == 3)
            origImage = img.imagedata3;

        // Apple each transform at a time to this channel
        iter = transforms->ch[channel - 1].begin();
        for (; iter != transforms->ch[channel - 1].end(); iter++)
            iter[0]->Execute(origImage, img.width, origImage, img.width, false);
    }
}

Image *Decoder::GetNewImage(const std::string &fileName, int channel) const {
    auto *temp = new Image();
    temp->ImageSetup(fileName);
    temp->channels = img.channels;
    temp->width = img.width;
    temp->height = img.height;
    temp->originalSize = img.originalSize;
    int sizeChannel = img.width * img.height;
    // Get according to channel number or all channels if number is zero
    if (img.channels >= 1 && (!channel || channel == 1))
        temp->SetChannelData(1, img.imagedata, sizeChannel);
    if (img.channels >= 2 && (!channel || channel == 2))
        temp->SetChannelData((!channel ? 2 : 1), img.imagedata2, sizeChannel);
    if (img.channels >= 3 && (!channel || channel == 3))
        temp->SetChannelData((!channel ? 3 : 1), img.imagedata3, sizeChannel);
    return temp;
}
