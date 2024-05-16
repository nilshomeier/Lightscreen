#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

// Define the number of LEDs
#define NUM_LEDS 30
#define LED_PIN 14

#define NUM_TOP_LEDS 10
#define NUM_BOTTOM_LEDS 10
#define NUM_SIDE_LEDS 5

uint8_t brightness = 0;
int state;

NeoGamma<NeoGammaTableMethod> colorGamma; // for any fade animations, best to correct gamma

// Create an instance of the Adafruit_NeoPixel class
NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> strip(NUM_LEDS, LED_PIN);

AnimEaseFunction easing;

#define colorSaturation 255

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

// Define the colors for each corner
RgbColor topLeftColor(255, 0, 0); // Red
RgbColor topRightColor(0, 255, 0); // Green
RgbColor bottomLeftColor(0, 0, 255); // Blue
RgbColor bottomRightColor(255, 255, 0); // Yellow

// Define the corner colors
// RgbColor topLeftColor, topRightColor, bottomRightColor, bottomLeftColor;
RgbColor newTopLeftColor, newTopRightColor, newBottomRightColor, newBottomLeftColor;

// NeoPixelAnimator instance
// NeoPixelAnimator animations(NUM_LEDS);

// NeoPixelAnimator animations(PixelCount, NEO_CENTISECONDS);
NeoPixelAnimator animations(NUM_LEDS, NEO_DECISECONDS);
// Possible values from 1 to 32768, and there some helpful constants defined as...
// NEO_MILLISECONDS        1    // ~65 seconds max duration, ms updates
// NEO_CENTISECONDS       10    // ~10.9 minutes max duration, centisecond updates
// NEO_DECISECONDS       100    // ~1.8 hours max duration, decisecond updates
// NEO_SECONDS          1000    // ~18.2 hours max duration, second updates
// NEO_DECASECONDS     10000    // ~7.5 days, 10 second updates

String rgbToString(RgbColor color) {
    return "{r:" + String(color.R) + ",g:" + String(color.G) + ",b:" + String(color.B) + "}";
}

// Function to calculate the color gradient between two colors
// RgbColor calculateGradient(const RgbColor& color1, const RgbColor& color2, float ratio) {
//     return RgbColor::LinearBlend(color1, color2, ratio);
// }

void generateNewCornerColors() {
    // Generate new random colors for each corner
    newTopLeftColor = white; //RgbColor(random(256), random(256), random(256));
    newTopRightColor = RgbColor(random(256), random(256), random(256));
    newBottomRightColor = RgbColor(random(256), random(256), random(256));
    newBottomLeftColor = RgbColor(random(256), random(256), random(256));
}

// Animation callback function
void animFadeTo(const AnimationParam& param) {
    if (param.state == AnimationState_Completed) {
        state = 2;
        Serial.print("End of animFadeTo at pixel 0: "); Serial.println(rgbToString(strip.GetPixelColor<RgbColor>(0))); 
    } else { 
        state = 1;
        easing = NeoEase::CubicInOut;

        float fadeAmount = easing(param.progress);
        Serial.print(fadeAmount);
        Serial.print(" -> "); Serial.print(brightness);

        // Calculate current colors for each corner based on transition progress
        RgbColor currentTopLeftColor = RgbColor::LinearBlend(topLeftColor, newTopLeftColor, fadeAmount);
        // Serial.print(" -> "); Serial.print(rgbToString(currentTopLeftColor));
        currentTopLeftColor = currentTopLeftColor.Dim(brightness);
        // Serial.print(" -> "); Serial.print(rgbToString(currentTopLeftColor));
        RgbColor currentTopRightColor = RgbColor::LinearBlend(topRightColor, newTopRightColor, fadeAmount);
        currentTopRightColor = currentTopRightColor.Dim(brightness);
        RgbColor currentBottomRightColor = RgbColor::LinearBlend(bottomRightColor, newBottomRightColor, fadeAmount);
        currentBottomRightColor = currentBottomRightColor.Dim(brightness);
        RgbColor currentBottomLeftColor = RgbColor::LinearBlend(bottomLeftColor, newBottomLeftColor, fadeAmount);
        currentBottomLeftColor = currentBottomLeftColor.Dim(brightness);

        // Serial.print(" -> Color Pixel 0: "); Serial.print(rgbToString(colorGamma.Correct(RgbColor::LinearBlend(currentTopLeftColor, currentTopRightColor, 0.0f / (NUM_TOP_LEDS - 1)))));
        Serial.print(" -> Color Pixel 0: "); Serial.print(rgbToString(RgbColor::LinearBlend(currentTopLeftColor, currentTopRightColor, 0.0f / (NUM_TOP_LEDS - 1))));
        Serial.println();
        
        // Top side gradient
        for (int i = 0; i < NUM_TOP_LEDS; i++) {
            strip.SetPixelColor(i, colorGamma.Correct(RgbColor::LinearBlend(currentTopLeftColor, currentTopRightColor, float(i) / (NUM_TOP_LEDS - 1))));
        }
        // Right side gradient
        for (int i = 0; i < NUM_SIDE_LEDS; i++) {
            strip.SetPixelColor(NUM_TOP_LEDS + i, colorGamma.Correct(RgbColor::LinearBlend(currentTopRightColor, currentBottomRightColor, float(i) / (NUM_SIDE_LEDS - 1))));
        }

        // Bottom side gradient
        for (int i = 0; i < NUM_BOTTOM_LEDS; i++) {
            strip.SetPixelColor(NUM_TOP_LEDS + NUM_SIDE_LEDS + i, colorGamma.Correct(RgbColor::LinearBlend(currentBottomRightColor, currentBottomLeftColor, float(i) / (NUM_BOTTOM_LEDS - 1))));
        }

        // Left side gradient
        for (int i = 0; i < NUM_SIDE_LEDS; i++) {
            strip.SetPixelColor(NUM_TOP_LEDS + NUM_SIDE_LEDS + NUM_BOTTOM_LEDS + i, colorGamma.Correct(RgbColor::LinearBlend(currentBottomLeftColor, currentTopLeftColor, float(i) / (NUM_SIDE_LEDS - 1))));
        }    

    }
}

void animFadeOff(const AnimationParam& param) {
    if (param.state == AnimationState_Completed) {
        state = 3;
    } else {
        state = 1;
        easing = NeoEase::CubicInOut;

        float fadeAmount = param.progress;
        Serial.print("Color pixel 0 at fadeAmount: "); Serial.print(fadeAmount);

        for (int i = 0; i < NUM_LEDS; i++) {
            // Serial.println(rgbToString(strip.GetPixelColor<RgbColor>(i))); 
            strip.SetPixelColor(i, colorGamma.Correct(RgbColor::LinearBlend(strip.GetPixelColor<RgbColor>(i), RgbColor(0), fadeAmount)));
        }

        Serial.print(" "); Serial.println(rgbToString(strip.GetPixelColor<RgbColor>(0))); 
        delay(100);

    }
}

void setup() {
    Serial.begin(115200);

    // Initialize the LED strip
    strip.Begin();
    strip.Show();

    state = 0;

    generateNewCornerColors();

    // Start the animation
    brightness = 255;
    animations.StartAnimation(0, 250, animFadeTo);
}

void loop() {

    if (animations.IsAnimating()) {
        // Update the animation
        animations.UpdateAnimations();
        strip.Show();
    } else {
        // if (state == 2) {
        //     Serial.print("Color pixel 0: "); Serial.println(rgbToString(strip.GetPixelColor<RgbColor>(0))); 
        //     delay(1000);
        //     animations.StartAnimation(0, 250, animFadeOff);
        // }
    }
}
