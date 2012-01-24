#pragma once

namespace z
{
    namespace Internal {
        class OptionBase
        {
        public:
            inline OptionBase(const z::string& sname, const z::string& lname, const z::string& desc)
            : _sname(sname), _lname(lname), _desc(desc){}

        public:
            virtual void handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite) = 0;
            inline  void show(std::ostream& str, const int& w = 32) const;

        private:
            z::string _sname;
            z::string _lname;
            z::string _desc;
        };

        template <typename T>
        class Option : public OptionBase
        {
        public:
            inline Option(const z::string& sname, const z::string& lname, const z::string& desc, T& val)
            : OptionBase(sname, lname, desc), _val(val){}

        public:
            virtual void handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite);

        private:
            T& _val;
        };

        inline void OptionBase::show(std::ostream& str, const int& w) const
        {
            z::string rv = _sname + ", " + _lname;
            rv += z::string(w - rv.size(), ' ');
            str << "  " << rv << _desc << std::endl;
        }

        template<>
        inline void Option<bool>::handle(z::list<z::string>::const_iterator& /*it*/, z::list<z::string>::const_iterator& /*ite*/)
        {
            _val = true;
        }

        template<>
        inline void Option<int>::handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite)
        {
            ++it;
            if(it != ite)
                _val = (*it).to<int>();
        }

        template<>
        inline void Option<z::string>::handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite)
        {
            ++it;
            if(it != ite)
                _val = *it;
        }

        template<>
        inline void Option< z::list<z::string> >::handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite)
        {
            ++it;
            if(it != ite)
                _val.add(*it);
        }

        template<>
        inline void Option< std::vector<z::string> >::handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite)
        {
            ++it;
            if(it != ite)
                _val.push_back(*it);
        }
    }

    class Parser
    {
    private:
        typedef z::olist<Internal::OptionBase> List_t;
        typedef z::dict<z::string, Internal::OptionBase* > Map_t;

        struct Command
        {
            inline Command() : _value(0){}
            z::string _desc;
            List_t _list;
            Map_t _map;
            int _value;
        };
        typedef z::odict<z::string, Command > CmdMap_t;

    public:
        Parser() : _hasCommands(false){}

    public:
        inline void addCommand(const z::string& cmd, int value, const z::string& desc);

        template <typename T>
        inline void addMap(Command& cmd, const z::string& sname, const z::string& lname, const z::string& desc, T& val);

        template <typename T>
        inline void add(const z::string& sname, const z::string& lname, const z::string& desc, T& val);

        template <typename T>
        inline void add(const z::string& cmd, const z::string& sname, const z::string& lname, const z::string& desc, T& val);

        inline const int& getCommand() const;
        inline const z::string& getError() const;

    public:
        inline int parse(int argc, char* argv[]);
        inline int parse(const z::list<z::string>& args);
        inline void show(std::ostream& str) const;

    private:
        inline bool           hasCommandMap(z::list<z::string>::const_iterator& it);
        inline const Command& getCommandMap(z::list<z::string>::const_iterator& it);

    private:
        CmdMap_t _cmdMap;
        bool _hasCommands;

    private:
        z::string _appName;
        int _command;
        z::string _error;
    };

    inline const int& Parser::getCommand() const
    {
        assert(_cmdMap.size() > 0);
        return _command;
    }

    inline const z::string& Parser::getError() const
    {
        return _error;
    }

    inline void Parser::addCommand(const z::string& cmd, int value, const z::string& desc)
    {
        assert(_cmdMap.find(cmd) == _cmdMap.end());
        _hasCommands = true;
        if(_cmdMap[cmd] == 0) {
            Command* nval = new Command();
            _cmdMap.set(cmd, nval);
        }
        Command& cmdd = _cmdMap.at(cmd);
        cmdd._desc = desc;
        cmdd._value = value;
    }

    template <typename T>
    inline void Parser::addMap(Command& cmd, const z::string& sname, const z::string& lname, const z::string& desc, T& val)
    {
        Internal::OptionBase* opt = new Internal::Option<T>(sname, lname, desc, val);
        cmd._list.add(opt);

        cmd._map[sname] = opt;
        cmd._map[lname] = opt;
    }

    template <typename T>
    inline void Parser::add(const z::string& cmd, const z::string& sname, const z::string& lname, const z::string& desc, T& val)
    {
        assert(_hasCommands);
        Command& cmdd = _cmdMap.at(cmd);
        addMap(cmdd, sname, lname, desc, val);
    }

    template <typename T>
    inline void Parser::add(const z::string& sname, const z::string& lname, const z::string& desc, T& val)
    {
        assert(!_hasCommands);
        Command& cmd = _cmdMap.at("");
        addMap(cmd, sname, lname, desc, val);
    }

    inline void Parser::show(std::ostream& str) const
    {
        if(_hasCommands)
        {
            str << _appName << " <command> <options> " << std::endl;
            str << "commands: ";
            z::string sep = "";
            for(CmdMap_t::const_iterator cit = _cmdMap.begin(), cite = _cmdMap.end(); cit != cite; ++cit)
            {
                str << sep << cit->first;
                sep = ", ";
            }
            str << std::endl;
            str << std::endl;

            for(CmdMap_t::const_iterator cit = _cmdMap.begin(), cite = _cmdMap.end(); cit != cite; ++cit)
            {
                const Command& cmd = z::ref(cit->second);
                str << cit->first << ": " << cmd._desc << std::endl;

                for(List_t::const_iterator it = cmd._list.begin(), ite = cmd._list.end(); it != ite; ++it)
                {
                    const Internal::OptionBase& opt = **it;
                    opt.show(str);
                }
                str << std::endl;
            }
        }else
        {
            str << _appName << " <options>" << std::endl;
            str << "options:" << std::endl;
            const Command& cmd  = _cmdMap.at("");
            for(List_t::const_iterator it = cmd._list.begin(), ite = cmd._list.end(); it != ite; ++it)
            {
                const Internal::OptionBase& opt = **it;
                opt.show(str);
            }
        }
    }

    inline bool Parser::hasCommandMap(z::list<z::string>::const_iterator& it)
    {
        if(_hasCommands)
        {
            if (!_cmdMap.has(*it))
                return false;
        }
        return true;
    }

    inline const Parser::Command& Parser::getCommandMap(z::list<z::string>::const_iterator& it)
    {
        if(_hasCommands)
        {
            z::string command = *it;
            assert(_cmdMap.has(command));
            ++it;
            return z::ref(_cmdMap[command]);
        }
        return z::ref(_cmdMap[""]);
    }

    inline int Parser::parse(const z::list<z::string>& args)
    {
        z::list<z::string>::const_iterator it = args.begin();
        z::list<z::string>::const_iterator ite = args.end();

        _appName = *it;
        ++it;
        while(it == ite)
            return 0;

        if (!hasCommandMap(it))
        {
            _error = "Invalid command: " + *it;
            return 1;
        }

        const Command& cmd = getCommandMap(it);
        _command = cmd._value;

        while(it != ite)
        {
            const z::string& str = *it;
            Map_t::const_iterator fit = cmd._map.find(str);
            if(fit != cmd._map.end())
            {
                Internal::OptionBase* opt = fit->second;
                opt->handle(it, ite);
            }else
            {
                _error = "Invalid option: " + str;
                return 1;
            }

            if(it != ite)
                ++it;
        }

        return 0;
    }

    inline int Parser::parse(int argc, char* argv[])
    {
        z::list<z::string> args;
        for(int i = 0; i < argc; ++i)
            args.add(argv[i]);
        return parse(args);
    }
}
