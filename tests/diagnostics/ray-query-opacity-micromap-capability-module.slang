//TEST_IGNORE_FILE:

public typealias MicromapFeatures = RayQueryFeatures<RayQueryOpacityMicroMaps>;

public uint declareQuery<Features : IRayQueryFeature>()
{
    RayQuery<RAY_FLAG_NONE, Features> query;
    return 1;
}
