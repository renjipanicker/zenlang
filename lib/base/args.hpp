#pragma once

namespace z
{
    namespace Internal {
        class OptionBase
        {
        public:
            inline OptionBase(const z::string& sname, const z::string& lname, const z::string& desc)
            : _sname(sname), _lname(lname), _desc(desc){}
            virtual ~OptionBase(){}

        public:
            virtual void handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite, const bool& inc) = 0;
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
            virtual void handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite, const bool& inc);

        private:
            T& _val;
        };

        inline void OptionBase::show(std::ostream& str, const int& w) const
        {
            z::string rv = _sname + ", " + _lname;
            rv += z::string(w - rv.size(), ' ');
            str << "  " << z::s2e(rv) << z::s2e(_desc) << std::endl;
        }

        template<>
        inline void Option<bool>::handle(z::list<z::string>::const_iterator& /*it*/, z::list<z::string>::const_iterator& /*ite*/, const bool& /*inc*/)
        {
            _val = true;
        }

        template<>
        inline void Option<int>::handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite, const bool& inc)
        {
            if(inc)
                ++it;
            if(it != ite)
                _val = (*it).to<int>();
        }

        template<>
        inline void Option<z::string>::handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite, const bool& inc)
        {
            if(inc)
                ++it;
            if(it != ite)
                _val = *it;
        }

        template<>
        inline void Option< z::list<z::string> >::handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite, const bool& inc)
        {
            if(inc)
                ++it;
            if(it != ite)
                _val.add(*it);
        }

        template<>
        inline void Option< std::vector<z::string> >::handle(z::list<z::string>::const_iterator& it, z::list<z::string>::const_iterator& ite, const bool& inc)
        {
            if(inc)
                ++it;
            if(it != ite)
                _val.push_back(*it);
        }
    }

    class ClParser
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
        ClParser() : _hasCommands(false), _command(0) {}

    public:
        inline ClParser::Command& getCommandMap(const z::string& cmd);
        inline void addCommand(const z::string& cmd, int value, const z::string& desc);

        template <typename T>
        inline void addMap(Command& cmd, const z::string& sname, const z::string& lname, const z::string& desc, T& val);

        template <typename T>
        inline void add(const z::string& cmd, const z::string& sname, const z::string& lname, const z::string& desc, T& val);

        template <typename T>
        inline void add(const z::string& sname, const z::string& lname, const z::string& desc, T& val);

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

    inline const int& ClParser::getCommand() const
    {
        assert(_cmdMap.size() > 0);
        return _command;
    }

    inline const z::string& ClParser::getError() const
    {
        return _error;
    }

    inline ClParser::Command& ClParser::getCommandMap(const z::string& cmd) {
        if(_cmdMap[cmd] == 0) {
            Command* nval = new Command();
            _cmdMap.set(cmd, nval);
        }
        return _cmdMap.at(cmd);
    }

    inline void ClParser::addCommand(const z::string& cmd, int value, const z::string& desc)
    {
        assert(_cmdMap.find(cmd) == _cmdMap.end());
        _hasCommands = true;
        Command& cmdd = getCommandMap(cmd);
        cmdd._desc = desc;
        cmdd._value = value;
    }

    template <typename T>
    inline void ClParser::addMap(Command& cmd, const z::string& sname, const z::string& lname, const z::string& desc, T& val)
    {
        Internal::OptionBase* opt = new Internal::Option<T>(sname, lname, desc, val);
        cmd._list.add(opt);

        cmd._map[sname] = opt;
        cmd._map[lname] = opt;
    }

    template <typename T>
    inline void ClParser::add(const z::string& cmd, const z::string& sname, const z::string& lname, const z::string& desc, T& val)
    {
        assert(_hasCommands);
        Command& cmdd = _cmdMap.at(cmd);
        addMap(cmdd, sname, lname, desc, val);
    }

    template <typename T>
    inline void ClParser::add(const z::string& sname, const z::string& lname, const z::string& desc, T& val)
    {
        assert(!_hasCommands);
        Command& cmd = getCommandMap("");
        addMap(cmd, sname, lname, desc, val);
    }

    inline void ClParser::show(std::ostream& str) const
    {
        if(_hasCommands)
        {
            str << z::s2e(_appName) << " <command> <options> " << std::endl;
            str << "commands: ";
            z::string sep = "";
            for(CmdMap_t::const_iterator cit = _cmdMap.begin(), cite = _cmdMap.end(); cit != cite; ++cit)
            {
                str << z::s2e(sep) << z::s2e(cit->first);
                sep = ", ";
            }
            str << std::endl;
            str << std::endl;

            for(CmdMap_t::const_iterator cit = _cmdMap.begin(), cite = _cmdMap.end(); cit != cite; ++cit)
            {
                const Command& cmd = z::ref(cit->second);
                str << z::s2e(cit->first) << ": " << z::s2e(cmd._desc) << std::endl;

                for(List_t::const_iterator it = cmd._list.begin(), ite = cmd._list.end(); it != ite; ++it)
                {
                    const Internal::OptionBase& opt = **it;
                    opt.show(str);
                }
                str << std::endl;
            }
        }else
        {
            str << z::s2e(_appName) << " <options>" << std::endl;
            str << "options:" << std::endl;
            const Command& cmd  = _cmdMap.at("");
            for(List_t::const_iterator it = cmd._list.begin(), ite = cmd._list.end(); it != ite; ++it)
            {
                const Internal::OptionBase& opt = **it;
                opt.show(str);
            }
        }
    }

    inline bool ClParser::hasCommandMap(z::list<z::string>::const_iterator& it)
    {
        if(_hasCommands) {
            z::string cmd = *it;
            if (_cmdMap.has(cmd))
                return true;
            if (_cmdMap.has("")) {
                return true;
            }
            return false;
        }
        return true;
    }

    inline const ClParser::Command& ClParser::getCommandMap(z::list<z::string>::const_iterator& it)
    {
        if(_hasCommands) {
            z::string cmd = *it;
            if(_cmdMap.has(cmd)) {
                ++it;
                return _cmdMap.at(cmd);
            }
            // else fall-thru...
        }
        assert(_cmdMap.has(""));
        return _cmdMap.at("");
    }

    inline int ClParser::parse(const z::list<z::string>& args)
    {
        z::list<z::string>::const_iterator it = args.begin();
        z::list<z::string>::const_iterator ite = args.end();

        _appName = *it;
        ++it;
        if(it == ite) {
            if(_cmdMap.has("")) {
                const Command& cmd = _cmdMap.at("");
                _command = cmd._value;
            } else {
                _command = 0;
            }

            return 0;
        }

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
            Map_t::const_iterator fit;
            bool inc = true;
            if(str.at(0) == '-') {
                fit = cmd._map.find(str);
            } else {
                fit = cmd._map.find("");
                inc = false;
            }

            if(fit == cmd._map.end()) {
#if defined(OSX)
                if(str.substr(0, 4) == "-psn") {
                    if(it != ite)
                        ++it;
                    continue;
                }
#endif
                _error = "Invalid option: " + str;
                return 1;
            }

            Internal::OptionBase* opt = fit->second;
            opt->handle(it, ite, inc);

            if(it != ite)
                ++it;
        }

        return 0;
    }

    inline int ClParser::parse(int argc, char* argv[])
    {
        z::list<z::string> args;
        for(int i = 0; i < argc; ++i)
            args.add(argv[i]);
        return parse(args);
    }
}
