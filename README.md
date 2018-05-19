## MAX9744 component for ESP-IDF

This is a very simple library for using the MAX9744 audio amplifier IC
from an ESP32 IDF project. These are the same amplifier ICs used on the
[Adafruit Stereo 20W Class D Audio Amplifier](https://www.adafruit.com/product/1752) breakout board.

### Installation

If you're using a normal ESP-IDF project, just clone this repository into
your `components` directory inside the project:

    cd components
    git clone https://github.com/infincia/max9744_espidf.git


However if you're using PlatformIO, you can alternatively add this repository
as a library in your platformio.ini:

    [env:development]
    platform = espressif32
    board = esp32dev
    framework = espidf
    lib_deps =
      https://github.com/infincia/max9744_espidf.git#v0.1.0

**Note:** I haven't tested this with PlatformIO at all, since it doesn't fully
support ESP-IDF v3.0 yet. This is partly why the library is not submitted to
the PlatformIO library registry at the moment.

### Setup

Connect the I2C wires to the GPIOs you have chosen to use, then run the
`make menuconfig` command from the root of your IDF project. The MAX9744
configuration will appear under the `components` section. You must enable
the component before it will be compiled at all, this is to allow excluding
it completely if necessary.

The only real settings are the GPIO numbers, which are 21 for SDA and 22 for
SCL, by default. Keep in mind those are the actual GPIO numbers used by the
ESP32 rather than pin numbers, so if your ESP32 board uses a custom number
scheme for the header pins you will have to figure out which GPIOs they are
actually connected to first.

Once the component is enabled all you need to do is import the correct header
and call the initialization function:

    #include <max9744_espidf.h>

    max9744_init();

The initialization function should only be called once, calling it in the
`app_main()` entry point for your project is a good idea.


### Control interface

The MAX9744 has a volume range of `0-63`, where `0` is essentially muted and
`63` is the maximum volume level. Note that the maximum volume level is
likely to cause distortion unless the power supply is at least 12v and 
capable of supplying enough power.

At power on, the MAX9744 defaults to a volume level of `40` out of `63`. 

The [datasheet](https://cdn-shop.adafruit.com/datasheets/MAX9744.pdf) says `40` corresponds to `-7.1dB`, but if that's meaningless 
to you, it's basically the equivalent of a slightly loud TV in a small room.

#### Set volume level

To set the volume to a specific level:

    #include <esp_log.h>
    static const char *TAG = "[MyProject]";

    #include <max9744_espidf.h>

    uint8_t volume = 30;

    if (ESP_OK != max9744_set_volume(volume)) {
        ESP_LOGE(TAG, "set volume failed");
    }

#### Increase/decrease volume

    #include <esp_log.h>
    static const char *TAG = "[MyProject]";

    #include <max9744_espidf.h>

    if (ESP_OK != max9744_increase_volume()) {
        ESP_LOGE(TAG, "increase volume failed");
    }

    if (ESP_OK != max9744_decrease_volume()) {
        ESP_LOGE(TAG, "decrease volume failed");
    }

You may notice that there is nothing that specifies what the volume level
was before calling these functions. This is because there is no I2C command
to read the *current* volume level from the amplifier.

It will be difficult to display or keep track of the volume level from 
your own code if you use the increase or decrease functions.

Instead, use a global variable or a local variable within a long running
FreeRTOS task to store the volume level, then change the variable and 
call `max9744_set_volume()` with it instead. This way your own code will 
always know what the current volume level is. You can wrap that up in your
own stateful volume increase/decrease functions to make things easier.


#### Set filterless modulation

    #include <esp_log.h>
    static const char *TAG = "[MyProject]";

    #include <max9744_espidf.h>

    if (ESP_OK != max9744_set_filterless_modulation()) {
        ESP_LOGE(TAG, "set filterless modulation failed");
    }

Sets the amplifier to filterless modulation mode (see the [datasheet](https://cdn-shop.adafruit.com/datasheets/MAX9744.pdf)). If
you need this function you likely already know what it does. As far as I
know, this is the default on the Adafruit 20w breakout.


#### Set classic PWM modulation

    #include <esp_log.h>
    static const char *TAG = "[MyProject]";

    #include <max9744_espidf.h>

    if (ESP_OK != max9744_set_classic_pwm_modulation()) {
        ESP_LOGE(TAG, "set classic pwm modulation failed");
    }

Sets the amplifier to classic PWM modulation mode (see the datasheet). If
you need this function you likely already know what it does.

The only thing I know about this setting is that it is not the default, and
using it will require additional EMI filtering on the speaker outputs. 

The datasheet suggests that classic PWM mode provides higher audio quality, but
I don't know how much it matters in practice.

### Why a library?

The MAX9744 has a very simple I2C protocol that allows changing the volume
level and configuring a few other settings.

The I2C command to set the volume level is idential to writing the level
as an unsigned 8-bit number directly over the I2C bus, but this is a quirk 
of the way the I2C command works.

Becauase the volume commands are so simple, technically you would not need
a separate library to call them, if that's all you want to do. However, 
ESP-IDF can be confusing for those who are coming from Arduino development 
where you would only need to call a few `Wire` functions. 

In ESP-IDF, things are a little more verbose as you need to manually configure 
the GPIOs for use with the I2C hardware channel before sending commands to 
an I2C device.

It makes the rest of your code more readable and maintainable to keep that 
stuff in a separate library, regardless of how simple the actual I2C commands
are or appear to be.
