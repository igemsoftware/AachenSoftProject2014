function [mask, seg] = automaticseeds(im)

    imc = im;

    %% to grayscale and filtering
    Z = double(rgb2gray(im));
    Z = 255 * Z / max(max(Z));

    filtertype = 'disk';
    Z = filter2(fspecial(filtertype), Z);
    Z = filter2(fspecial(filtertype), filter2(fspecial(filtertype), Z));
    Z = 255 * Z / max(max(Z));   
    
    %% calculating similarity score/smoothness index
    k=4;
    sSI = similarity(Z,k);
    sSI = sSI / max(max(sSI));  
    
    %% classify
    pathogene = ((sSI > 0.85) == 1) & ((Z > 235) == 1);  

    mask = ones( size(imc) );
    seg = zeros( size(imc) );

    
    %% output
    for i=1:size(im,1)
        for j=1:size(im,2)
            
            if (pathogene(i,j) == 1)
                seg(i,j,1:3) = [255 0 0];
                mask(i, j, 1:3) = [0 0 0];
            end
        end
    end
end