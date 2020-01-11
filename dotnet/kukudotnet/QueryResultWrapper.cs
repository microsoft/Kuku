using System;

namespace KukuDotNet
{
    public class QueryResultWrapper
    {
        private readonly QueryResult _resultStruct;

        internal QueryResultWrapper(QueryResult resultStruct)
        {
            this._resultStruct = resultStruct;
        }

        public uint Location => this._resultStruct.location;

        public uint LocFuncIndex => this._resultStruct.loc_func_index;

        public bool InStash => this._resultStruct.in_stash; 
    }
}