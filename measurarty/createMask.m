% Auto-generated by colorThresholder app on 15-Oct-2014
%-------------------------------------------------------
function [maskedRGBImage] = createMask(srmimg)
RGB = srmimg;

% Convert RGB image to chosen color space
I = rgb2hsv(RGB);

% Define thresholds for channel 1 based on histogram settings
channel1Min = 0.462;
channel1Max = 0.520;

% Define thresholds for channel 2 based on histogram settings
channel2Min = 0.99;
channel2Max = 1.000;

% Define thresholds for channel 3 based on histogram settings
channel3Min = 0.25;
channel3Max = 0.32;

% Create mask based on chosen histogram thresholds
BW = (I(:,:,1) >= channel1Min ) & (I(:,:,1) <= channel1Max) & ...
    (I(:,:,2) >= channel2Min ) & (I(:,:,2) <= channel2Max) & ...
    (I(:,:,3) >= channel3Min ) & (I(:,:,3) <= channel3Max);

% Initialize output masked image based on input image.
maskedRGBImage = RGB;

% Set background pixels where BW is false to zero.
maskedRGBImage(repmat(~BW,[1 1 3])) = 0;

%figure();
%imshow(maskedRGBImage);

end