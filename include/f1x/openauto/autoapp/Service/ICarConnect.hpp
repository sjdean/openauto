#pragma once

namespace f1x
{
    namespace openauto
    {
        namespace autoapp
        {
            namespace service
            {
                class ICarConnect
                {
                public:
                    typedef std::shared_ptr<ICarConnect> Pointer;
                    virtual ~ICarConnect() = default;
                };

            }
        }
    }
}
