function [ S ] = similarity( I, k )
%% similarity calculates similarity of kxk area
%  uses jitter measure presented in
%  Joppich M, Rausch D, Kuhlen T. Adaptive human motion prediction using multiple model approaches. In: Virtuelle und Erweiterte Realität, 10. Workshop der GI-Fachgruppe VR/AR. Shaker Verlag; 2013. p. 169–80.
%  first time implemented as standalone measure here by Joppich M

    function v = calcsim(Img, iIu, iIv, i,j, k)
        
        v = 0;
        
        for s=i-k:i+k
            for t=j-k:j+k
                ms = min(max(1, s), size(Img,1));
                mt = min(max(1, t), size(Img,2));
                v = v + abs(iIu(ms,mt)) + abs(iIv(ms,mt));
            end
        end
        
    end

    S = zeros([size(I,1), size(I,2)]);

    [Iu,Iv] = gradient(I);

    for m=1:size(I,1)
        for n=1:size(I,2)
            %calculates score
            S(m,n) = calcsim(I, Iu, Iv, m, n, k);
        end
    end
    
    S = max(max(S)) - S;

end

