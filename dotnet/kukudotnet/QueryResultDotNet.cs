namespace KukuDotNet
{
    public class QueryResultDotNet
    {
        private readonly QueryResult _resultStruct;

        internal QueryResultDotNet(QueryResult resultStruct)
        {
            this._resultStruct = resultStruct;
        }

        public long Location => this._resultStruct.location;

        public long LocFuncIndex => this._resultStruct.loc_func_index;

        public bool InStash => this._resultStruct.in_stash;
    }
}