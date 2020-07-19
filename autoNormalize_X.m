function [X,out] = autoNormalize_X(X, opts)

%% This code is taken from https://github.com/andremun/InstanceSpace with a little modification

nfeats = size(X,2);
if opts.flag
    disp('-> Auto-normalizing the data using Box-Cox and Z transformations.');
    out.minX = min(X,[],1);
    X = bsxfun(@minus,X,out.minX)+1;
    out.lambdaX = zeros(1,nfeats);
    out.muX = zeros(1,nfeats);
    out.sigmaX = zeros(1,nfeats);
    for i=1:nfeats
        aux = X(:,i);
        idx = isnan(aux);
        [aux, out.lambdaX(i)] = boxcox(aux(~idx));
        [aux, out.muX(i), out.sigmaX(i)] = zscore(aux);
        X(~idx,i) = aux;
    end

else
    out.lambdaX = ones(1,nfeats);
    out.muX = zeros(1,nfeats);
    out.sigmaX = ones(1,nfeats);
end

end