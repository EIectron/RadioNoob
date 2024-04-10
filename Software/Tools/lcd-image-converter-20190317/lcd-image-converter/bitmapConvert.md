## How to convert pictures for monochrome sharp memory display?


![](./Documents/AI-size.PNG)
1.) The dimensions of your work in the visual design program should not be larger than 400x240 pixels.

![](./Documents/conversion_setting.PNG)
2.) And open the image .jpeg or .png doesn't matter. Enter the conversions setting from the options tab

![](./Documents/conversion-import.PNG)
3. Import to Monochrome.xml file.

![](./Documents/convert.PNG)
4. Convert image

![](./Documents/save.PNG)
5. File settings must be `C source code(*.c)`

![](./Documents/add-lib.PNG)
6. After open the converted file and add to up side `#include <tImage.h>`

![](./Documents/file-name.PNG)
7. This is c file name

![](./Documents/add-memory-lib.PNG)
8. It is ready for use after adding it to the memory_lcd.h file in the Library folder as shown in the figure.

ex. 
``` C
GFXDisplayPutImage(0, 0, &splash_screen, false);
```

