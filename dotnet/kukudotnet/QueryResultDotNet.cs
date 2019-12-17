using System;

namespace KukuDotNet
{
    public class QueryResultDotNet
    {
        private readonly QueryResult _resultStruct;

        internal QueryResultDotNet(QueryResult resultStruct)
        {
            this._resultStruct = resultStruct;
        }

        public ulong Location => this._resultStruct.location;

        public ulong LocFuncIndex => this._resultStruct.loc_func_index;

        public bool InStash => this._resultStruct.in_stash; 
    }
}