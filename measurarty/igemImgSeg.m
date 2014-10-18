function [segRGB, marked] = igemImgSeg(RGB)

%% First load the desired image
image=double(RGB);


%% Secondly process the image by SRM - only 1 pass
Qlevels = 256;
[maps,images]=singlesrm(double(image),Qlevels);
Iedge=zeros([size(images,1),size(images,2)]);
%these are artefacts from the code from http://www.mathworks.com/matlabcentral/fileexchange/25619-image-segmentation-using-statistical-region-merging
map=reshape(maps,size(Iedge));
srmedImg = uint8(images);

%% HSV segment the SRM result
maskedImg = createMask( srmedImg );

%% automatic segment HSV segment for signal

[mask, seg] = automaticseeds(maskedImg);
%some conversion stuff so matlab does not get unhappy :)
segRGB = uint8(mask) .* RGB;
segRGB = segRGB + uint8(seg);

marked =  sum(sum( sum(mask == 0))) / 3;
end
