#pragma once

#include "../core/Core.h"
#include <vector>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <bitset>
#include <any>

namespace EnGl
{
    template<typename T>
    concept ECSComponent =
        std::is_default_constructible_v<T> &&
        std::is_move_constructible_v<T> &&
        std::is_move_assignable_v<T>
    ;

    using Entity = u32;

    template<ECSComponent... AllCs>
    struct Ecs
    {
        static constexpr size_t NComponents = sizeof...(AllCs);
        using CSignature = std::bitset<NComponents>;

        class ComponentStorage
        {
        public:
            struct CreationInfo
            {
                CSignature Signature;
            };

            template<ECSComponent... Cs>
            CreationInfo Create(Entity id)
            {
                auto signature = GetSignature<Cs...>();
                ArchetypeInfo& arch = GetArchetypeStorage(signature);

                if (arch.Size < arch.Entities.size())
                {
                    ((std::get<std::vector<Cs>>(arch.Components)[arch.Size] = Cs{}), ...);
                    arch.Entities[arch.Size] = id;
                }
                else
                {
                    ((std::get<std::vector<Cs>>(arch.Components).emplace_back()), ...);
                    arch.Entities.push_back(id);
                }
                arch.GlobalToLocal[id] = arch.Size;
                arch.Size++;

                return { .Signature = signature };
            }

            template<ECSComponent... Cs, typename F>
            CreationInfo Create(Entity id, F&& initializer)
            {
                auto signature = GetSignature<Cs...>();
                ArchetypeInfo& arch = GetArchetypeStorage(signature);

                std::tuple<Cs...> components{};
                std::apply(std::forward<F>(initializer), components);

                if (arch.Size < arch.Entities.size())
                {
                    ((std::get<std::vector<Cs>>(arch.Components)[arch.Size] = std::get<Cs>(components)), ...);
                    arch.Entities[arch.Size] = id;
                }
                else
                {
                    ((std::get<std::vector<Cs>>(arch.Components).push_back(std::get<Cs>(components))), ...);
                    arch.Entities.push_back(id);
                }
                
                arch.GlobalToLocal[id] = arch.Size;
                arch.Size++;

                return { .Signature = signature };
            }

            template<ECSComponent... Cs>
            CreationInfo Add(Entity id, const CSignature& prevSignature)
            {
                auto signature = GetSignature<Cs...>();
                ArchetypeInfo& prevArch = GetArchetypeStorage(prevSignature);
                signature |= prevSignature;
                ArchetypeInfo& arch = GetArchetypeStorage(signature);

                bool reuse = arch.Size < arch.Entities.size();
               
                auto addNew = [&]<ECSComponent C>()
                {
                    if (reuse)
                    {
                        std::get<std::vector<C>>(arch.Components)[arch.Size] = C{};
                    }
                    else
                    {
                        std::get<std::vector<C>>(arch.Components).emplace_back();
                    }
                };

                auto copyPrev = [&]<ECSComponent C>()
                {
                    if (!prevSignature[GetComponentId<C>()]) return;

                    if (reuse)
                    {
                        std::get<std::vector<C>>(arch.Components)[arch.Size] = std::get<std::vector<C>>(prevArch.Components)[prevArch.GlobalToLocal[id]];
                    }
                    else
                    {
                        std::get<std::vector<C>>(arch.Components).push_back(std::get<std::vector<C>>(prevArch.Components)[prevArch.GlobalToLocal[id]]);
                    }
                };

                if (reuse) arch.Entities[arch.Size] = id;
                else arch.Entities.push_back(id);

                (addNew.template operator() <Cs>(), ...);
                (copyPrev.template operator() <AllCs>(), ...);

                Remove(id, prevSignature);
                arch.GlobalToLocal[id] = arch.Size;
                arch.Size++;

                return { .Signature = signature };
            }

            template<ECSComponent... Cs, typename F>
            CreationInfo Add(Entity id, const CSignature& prevSignature, F&& initializer)
            {
                auto signature = GetSignature<Cs...>();
                ArchetypeInfo& prevArch = GetArchetypeStorage(prevSignature);
                signature |= prevSignature;
                ArchetypeInfo& arch = GetArchetypeStorage(signature);

                std::tuple<Cs...> newComponents{};
                std::apply(std::forward<F>(initializer), newComponents);

                bool reuse = arch.Size < arch.Entities.size();

                auto addNew = [&]<ECSComponent C>()
                {
                    if (reuse)
                    {
                        std::get<std::vector<C>>(arch.Components)[arch.Size] = std::get<C>(newComponents);
                    }
                    else
                    {
                        std::get<std::vector<C>>(arch.Components).emplace_back(std::move(std::get<C>(newComponents)));
                    }
                };

                auto copyPrev = [&]<ECSComponent C>()
                {
                    if (!prevSignature[GetComponentId<C>()]) return;

                    if (reuse)
                    {
                        std::get<std::vector<C>>(arch.Components)[arch.Size] = std::get<std::vector<C>>(prevArch.Components)[prevArch.GlobalToLocal[id]];
                    }
                    else
                    {
                        std::get<std::vector<C>>(arch.Components).push_back(std::get<std::vector<C>>(prevArch.Components)[prevArch.GlobalToLocal[id]]);
                    }
                };

                if (reuse) arch.Entities[arch.Size] = id;
                else arch.Entities.push_back(id);

                (addNew.template operator() <Cs>(), ...);
                (copyPrev.template operator() <AllCs>(), ...);

                Remove(id, prevSignature);
                arch.GlobalToLocal[id] = arch.Size;
                arch.Size++;

                return { .Signature = signature };
            }

            template<ECSComponent... Cs>
            CreationInfo Remove(Entity id, const CSignature& prevSignature)
            {
                auto signature = GetSignature<Cs...>();
                ArchetypeInfo& prevArch = GetArchetypeStorage(prevSignature);

                signature = prevSignature & ~signature;

                ArchetypeInfo& arch = GetArchetypeStorage(signature);

                bool reuse = arch.Size < arch.Entities.size();

                auto copyMatching = [&]<ECSComponent C>()
                {
                    if (!signature[GetComponentId<C>()]) return;

                    if (reuse)
                    {
                        std::get<std::vector<C>>(arch.Components)[arch.Size] = std::get<std::vector<C>>(prevArch.Components)[prevArch.GlobalToLocal[id]];
                    }
                    else
                    {
                        std::get<std::vector<C>>(arch.Components).push_back(std::get<std::vector<C>>(prevArch.Components)[prevArch.GlobalToLocal[id]]);
                    }
                };

                if (reuse) arch.Entities[arch.Size] = id;
                else arch.Entities.push_back(id);

                (copyMatching.template operator() <AllCs>(), ...);

                Remove(id, prevSignature);
                arch.GlobalToLocal[id] = arch.Size;
                arch.Size++;

                return { .Signature = signature };
            }

            template<ECSComponent C1, ECSComponent C2, ECSComponent... Cs>
            std::tuple<C1&, C2&, Cs&...> Get(Entity entity, const CSignature& signature)
            {
                ArchetypeInfo& arch = m_Archetypes[signature];
                
                Entity localId = arch.GlobalToLocal[entity];

                return std::tie(
                    std::get<std::vector<C1>>(arch.Components)[localId],
                    std::get<std::vector<C2>>(arch.Components)[localId],
                    std::get<std::vector<Cs>>(arch.Components)[localId]...
                );
            }

            template<ECSComponent C>
            C& Get(Entity entity, const CSignature& signature)
            {
                ArchetypeInfo& arch = m_Archetypes[signature];

                Entity localId = arch.GlobalToLocal[entity];

                return std::get<std::vector<C>>(arch.Components)[localId];
            }

            template<ECSComponent Cs>
            bool Has(const CSignature& signature) const
            {
                return signature[GetComponentId<Cs>()];
            }

            void Remove(Entity id, const CSignature& signature)
            {
                ArchetypeInfo& arch = m_Archetypes[signature];

                Entity localId = arch.GlobalToLocal.at(id);
                Entity back = arch.Size - 1;
                Entity moved = arch.Entities[back];

                arch.GlobalToLocal.erase(id);
                if (localId != back)
                {
                    std::swap(arch.Entities[localId], arch.Entities[back]);

                    std::apply([localId, back](auto&... vectors) -> void
                        {
                            ((vectors.empty() ? void() : std::swap(vectors[localId], vectors[back])), ...);
                        }
                    , arch.Components);

                    arch.GlobalToLocal[moved] = localId;
                }
                
                arch.Size--;
            }

            template<ECSComponent... Cs>
            u32 CountAll() const
            {
                u32 count = 0;

                CSignature signature = GetSignature<Cs...>();

                for (const auto& [archSignature, arch] : m_Archetypes)
                {
                    if ((archSignature & signature) != signature) continue;

                    (( count += static_cast<Entity>(std::get<std::vector<Cs>>(arch.Components).size()) ), ...);
                }

                return count;
            }

            template<ECSComponent C>
            constexpr u32 GetComponentId() const
            {
                u32 id = 0;
                ((std::is_same_v<AllCs, C> ? false : (id++, true)) && ...);
                return id;
            }

            template<ECSComponent... Cs>
            CSignature GetSignature() const
            {
                CSignature signature;
                ((signature[GetComponentId<Cs>()] = true), ...);
                return signature;
            }

        private:
            struct ArchetypeInfo
            {
                Entity Size = 0;
                std::vector<Entity> Entities;
                std::tuple<std::vector<AllCs>...> Components;
                std::unordered_map<Entity, Entity> GlobalToLocal;
                u32 Id = 0;
            };

            ArchetypeInfo& GetArchetypeStorage(const CSignature& signature)
            {
                return m_Archetypes[signature];
            };

            template<ECSComponent... Cs>
            struct ArchIterator
            {
                ArchIterator(ComponentStorage* storage, ArchetypeInfo* arch, size_t curr)
                    : storage(storage), arch(arch), curr(curr) {
                }

                ArchIterator() = default;

                ArchIterator<Cs...>& operator++()
                {
                    curr++;
                    return *this;
                }

                ArchIterator<Cs...> operator++(int)
                {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                bool operator!=(const ArchIterator& other) const { return curr != other.curr; }
                bool operator==(const ArchIterator& other) const { return curr == other.curr; }

                auto operator*()
                {
                    return std::tie(
                        arch->Entities[curr],
                        std::get<std::vector<Cs>>(arch->Components)[curr]...
                    );
                }

            private:
                ComponentStorage* storage = nullptr;
                ArchetypeInfo* arch = nullptr;
                size_t curr = 0;
            };

            template<ECSComponent... Cs>
            struct ArchIteratorStruct
            {
                ArchIteratorStruct(ComponentStorage* storage, ArchetypeInfo* arch)
                    : storage(storage), arch(arch) {
                }

                ArchIteratorStruct() = default;

                ArchIterator<Cs...> begin() { return ArchIterator<Cs...>{storage, arch, 0}; }
                ArchIterator<Cs...> end() { return ArchIterator<Cs...>{storage, arch, arch->Size}; }

            private:
                ComponentStorage* storage = nullptr;
                ArchetypeInfo* arch = nullptr;
            };

            template<ECSComponent C>
            std::queue<Entity>& GetDeadComponentStorage() const
            {
                static std::queue<Entity> storage;
                return storage;
            };

            std::unordered_map<CSignature, ArchetypeInfo> m_Archetypes;
            u32 m_TotalComponents = 0;

        public:
            template<ECSComponent... Cs>
            struct QueryIterator
            {
                using iter = std::unordered_map<CSignature, ArchetypeInfo>::iterator;

                QueryIterator(ComponentStorage* storage, CSignature signature, CSignature excludeSignature, iter c = 0)
                    : storage(storage), signature(std::move(signature)), excludeSignature(std::move(excludeSignature))
                {
                    iCurr = c;
                    SkipInvalid();
                }

                template<ECSComponent... CEs>
                QueryIterator<Cs...>& Without()
                {
                    excludeSignature = storage->GetSignature<CEs...>();
                    return *this;
                }

                QueryIterator<Cs...>& operator++()
                {
                    ++curr;
                    if (curr == end)
                    {
                        ++iCurr;
                        SkipInvalid();
                    }
                    return *this;
                }

                QueryIterator<Cs...> operator++(int)
                {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                bool operator!=(const QueryIterator& other) const { return iCurr != other.iCurr; }
                bool operator==(const QueryIterator& other) const { return iCurr == other.iCurr; }

                auto operator*() { return *curr; }

            private:
                void SkipInvalid()
                {
                    while (iCurr != storage->m_Archetypes.end())
                    {
                        auto arch = &(*iCurr).second;
                        auto archSignature = (*iCurr).first;
                        bool matches = ((archSignature & signature) == signature);
                        bool dontExclude = (archSignature & excludeSignature).none();

                        if (arch->Size == 0 || !matches || !dontExclude)
                        {
                            ++iCurr;
                        }
                        else
                        {
                            currIter = ArchIteratorStruct<Cs...>{ storage, arch };
                            curr = currIter.begin();
                            end = currIter.end();
                            break;
                        }
                    }
                }

                CSignature signature;
                CSignature excludeSignature;
                ComponentStorage* storage = nullptr;
                iter iCurr;
                ArchIterator<Cs...> curr;
                ArchIterator<Cs...> end;
                ArchIteratorStruct<Cs...> currIter;
            };

            template<ECSComponent... Cs>
            struct QueryStruct
            {
                QueryStruct(ComponentStorage* storage)
                    : storage(storage), signature(storage->GetSignature<Cs...>()) {
                }

                QueryIterator<Cs...> begin()
                {
                    return QueryIterator<Cs...>{storage, signature, excludeSignature, storage->m_Archetypes.begin()};
                }

                QueryIterator<Cs...> end()
                {
                    return QueryIterator<Cs...>{storage, signature, excludeSignature, storage->m_Archetypes.end()};
                }

                template<ECSComponent... CsE>
                QueryStruct<Cs...>& Exclude()
                {
                    excludeSignature = storage->GetSignature<CsE...>();
                    return *this;
                }

            private:
                Ecs<AllCs...>::ComponentStorage* storage;
                CSignature signature;
                CSignature excludeSignature;
            };

            template<ECSComponent... Cs>
            QueryStruct<Cs...> Query() { return QueryStruct<Cs...>{this}; }
        };

        class EntityManager
        {
        public:

            template<ECSComponent... Cs>
            CSignature GetSignature()
            {
                return m_ComponentStorage.GetSignature<Cs...>();
            }

            template<ECSComponent... Cs>
            Entity Create(std::string name = "Entity")
            {
                Entity id = NextId();
                auto res = m_ComponentStorage.Create<Cs...>(id);

                (id == m_Entities.size()) ? NewEntity(res, std::move(name)) : ReuseEntity(res, std::move(name), id);
                
                Size++;
                return id;
            }

            template<ECSComponent... Cs, typename F>
            Entity Create(F&& initializer, std::string name = "Entity")
            {
                Entity id = NextId();
                auto res = m_ComponentStorage.Create<Cs...>(id, std::forward<F>(initializer));

                (id == m_Entities.size()) ? NewEntity(res, std::move(name)) : ReuseEntity(res, std::move(name), id);
                Size++;

                return id;
            }

            template<ECSComponent... Cs>
            void Add(Entity id)
            {
                auto res = m_ComponentStorage.Add<Cs...>(id, m_Signatures[id]);

                m_Signatures[id] = res.Signature;
            }

            template<ECSComponent... Cs, typename F>
            void Add(Entity id, F&& initializer)
            {
                auto res = m_ComponentStorage.Add<Cs...>(id, m_Signatures[id], std::forward<F>(initializer));

                m_Signatures[id] = res.Signature;
            }

            template<ECSComponent... Cs>
            void Remove(Entity id)
            {
                auto res = m_ComponentStorage.Remove<Cs...>(id, m_Signatures[id]);

                m_Signatures[id] = res.Signature;
            }

            template<ECSComponent C1, ECSComponent C2, ECSComponent... Cs>
            std::tuple<C1&, C2&, Cs&...> Get(Entity id)
            {
                return m_ComponentStorage.Get<C1, C2, Cs...>(id, m_Signatures[id]);
            }

            template<ECSComponent C>
            C& Get(Entity id)
            {
                return m_ComponentStorage.Get<C>(id, m_Signatures[id]);
            }

            const std::string& GetName(Entity id) const
            {
                return m_Names[id];
            }

            template<ECSComponent Cs>
            bool Has(Entity id) const
            {
                return m_ComponentStorage.Has<Cs>(m_Signatures[id]);
            }

            void Remove(Entity id)
            {
                if (!m_Entities[id]) return;

                m_ComponentStorage.Remove(id, m_Signatures[id]);
                m_DeadIds.push(id);
                m_Entities[id] = false;
                Size--;
            }

            u32 Count() const
            {
                return Size;
            }

            template<ECSComponent... Cs>
            u32 CountAll() const
            {
                return m_ComponentStorage.CountAll<Cs...>();
            }

            template<ECSComponent... Cs>
            Ecs<AllCs...>::ComponentStorage::QueryStruct<Cs...> Query()
            {
                return m_ComponentStorage.Query<Cs...>();
            }

            struct Iterator
            {
                Iterator(EntityManager* manager, const CSignature& signature, const CSignature& excludeSignature = {}, Entity c = 0) : Manager(manager), Signature(signature), ExcludeSignature(signature), Curr(c)
                {
                    SkipInvalid();
                }

                Iterator Exclude(const CSignature& excludeSignature)
                {
                    return Iterator{ Manager, Signature, excludeSignature, Curr };
                }

                Iterator begin()
                {
                    return Iterator(Manager, Signature, ExcludeSignature, 0);
                }

                Iterator end()
                {
                    return Iterator(Manager, Signature, ExcludeSignature, static_cast<Entity>(Manager->m_Entities.size()));
                }

                void SkipInvalid()
                {
                    while (
                        Curr < Manager->m_Entities.size() && (!Manager->m_Entities[Curr] ||
                        (((Signature & Manager->m_Signatures[Curr]) == Signature) &&
                        (Signature & ExcludeSignature).none()))
                        )
                    {
                        Curr++;
                    }
                }

                Iterator& operator++()
                {
                    Curr++;
                    SkipInvalid();
                    return *this;
                }

                Iterator operator++(int)
                {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                bool operator==(const Iterator& other) const
                {
                    return Curr == other.Curr && Signature == other.Signature && ExcludeSignature == other.ExcludeSignature;
                }

                bool operator!=(const Iterator& other) const
                {
                    return !(*this == other);
                }

                Entity operator*()
                {
                    return Curr;
                }

                const CSignature& Signature;
                const CSignature& ExcludeSignature;
                EntityManager* Manager = nullptr;
            private:
                Entity Curr = 0;
            };

            Iterator Query(const CSignature& signature)
            {
                return Iterator{ this, signature };
            }

        private:
            void NewEntity(Ecs<AllCs...>::ComponentStorage::CreationInfo& res, std::string&& name)
            {
                m_Entities.push_back(true);
                m_Signatures.push_back(res.Signature);
                m_Names.push_back(std::move(name));
                m_Generations.push_back(0);
            }

            void ReuseEntity(Ecs<AllCs...>::ComponentStorage::CreationInfo& res, std::string&& name, Entity id)
            {
                m_Entities[id] = true;
                m_Signatures[id] = res.Signature;
                m_Names[id] = std::move(name);
                m_Generations[id]++;
            }

            Entity NextId()
            {
                if (!m_DeadIds.empty())
                {
                    Entity id = m_DeadIds.front();
                    m_DeadIds.pop();
                    return id;
                }

                return static_cast<Entity>(m_Entities.size());
            }

            u32 Size = 0;
            std::queue<Entity> m_DeadIds;
            std::vector<bool> m_Entities{false};
            std::vector<std::string> m_Names{"default"};
            std::vector<u32> m_Generations{0};
            std::vector<CSignature> m_Signatures{0};
            Ecs<AllCs...>::ComponentStorage m_ComponentStorage{};
        };

        template<typename Context>
        class SystemRegistry;

        template<typename Context>
        class System
        {
        public:
            virtual void Run(Ecs<AllCs...>::EntityManager& manager, Context& context) {};
            virtual void Init(Ecs<AllCs...>::EntityManager& manager) {};
            virtual void Editor(Ecs<AllCs...>::EntityManager& manager, Context& context) {};
            virtual ~System() = default;
        private:
            friend class SystemRegistry<Context>;
        };

        template<typename Context>
        class SystemRegistry
        {
        public:
            SystemRegistry(Ecs<AllCs...>::EntityManager& manager) : m_Manager(manager) {}

            template<typename T, typename... Args>
                requires std::derived_from<T, typename Ecs<AllCs...>::template System<Context>>
            SystemRegistry& Add(Args&&... args)
            {
                m_Systems.push_back(make_scope<T>(std::forward<Args>(args)...));
                m_Names.push_back(typeid(T).name());
                return *this;
            }

            void Init()
            {
                for (auto& system : m_Systems)
                {
                    system->Init(m_Manager);
                }
            }

            void Run(Context& context)
            {
                for (auto& system : m_Systems)
                {
                    system->Run(m_Manager, context);
                }
            }

            template<typename F>
            void Run(Context& context, F&& func)
            {
                for (auto& system : m_Systems)
                {
                    func([&]() { system->Run(m_Manager, context); });
                }
            }

            const auto& Systems() const
            {
                return m_Systems;
            }

            System<Context>* Get(size_t idx) const
            {
                return m_Systems[idx].get();
            }

            const std::string& GetName(size_t idx) const
            {
                return m_Names[idx];
            }

        private:
            std::vector< scope< typename Ecs<AllCs...>::template System<Context> > > m_Systems;
            std::vector< std::string > m_Names;
            Ecs<AllCs...>::EntityManager& m_Manager;
        };

    private:
        Ecs<AllCs...>::EntityManager m_Entity{};
    public:
        template<typename Context>
        Ecs<AllCs...>::SystemRegistry<Context>& eSystem() { 
            static typename Ecs<AllCs...>::template SystemRegistry<Context> systemRegistry { m_Entity };
            return systemRegistry;
        }

        Ecs<AllCs...>::EntityManager& eEntity() { return m_Entity; }
    };
}
