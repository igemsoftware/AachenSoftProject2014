%% automatic image analyser demo
% will automatically segment all .jpg files from the selected dir.
directory = uigetdir;
files = dir(fullfile(directory,'*.jpg'));

for k = 1:length(files)
    filename = files(k).name;

    %filename = 'K131026_pseudomonas_120';
    filein = filename;%strcat( filename, '.jpg');

    RGB = imread( filein );

    [segIMG, marked] = igemImgSeg(RGB);

    fileout = strcat( 'seg_', num2str(marked),'_', filename, '.png');

    %imshow(segIMG);
    imwrite(segIMG, fileout , 'png');
  
end

