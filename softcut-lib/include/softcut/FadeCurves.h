//
// Created by ezra on 11/15/18.
//
// static class for producing curves in fade period
//
// FIXME: this should be an object owned by SoftcutHead, passed to child SubHeads


#ifndef Softcut_FADECURVES_H
#define Softcut_FADECURVES_H

namespace softcut {

    class FadeCurves {
    public:
        typedef enum { Linear=0, Sine=1, Raised=2 } Shape;

        // initialize with defaults
        void init();
         void setRecDelayRatio(float x);
         void setPreWindowRatio(float x);
         void setMinRecDelayFrames(unsigned int x);
         void setMinPreWindowFrames(unsigned int x);
        // set curve shape
         void setPreShape(Shape x);
         void setRecShape(Shape x);
        // x is assumed to be in [0,1]
         float getRecFadeValue(float x);

         float getPreFadeValue(float x);

    private:
         void calcPreFade();
         void calcRecFade();

    private:

        // xfade curve buffers
        static constexpr unsigned int fadeBufSize = 1001;

        // record delay and pre window in fade, as proportion of fade time
         float recDelayRatio;
         float preWindowRatio;
        // minimum record delay/pre window, in frames
         unsigned int recDelayMinFrames;
         unsigned int preWindowMinFrames;
         float recFadeBuf[fadeBufSize];
         float preFadeBuf[fadeBufSize];
         Shape recShape;
         Shape preShape;
    };
}

#endif //Softcut_FADECURVES_H
