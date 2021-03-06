{% extends 'interface_base.cpp' %}

{% set has_prepare_prototype_and_interface_object =
    unscopeables or has_conditional_attributes_on_prototype or
    methods | conditionally_exposed(is_partial) %}
{% set prepare_prototype_and_interface_object_func =
    '%s::preparePrototypeAndInterfaceObject' % v8_class_or_partial
    if has_prepare_prototype_and_interface_object
    else 'nullptr' %}


{##############################################################################}
{% block prepare_prototype_and_interface_object %}
{% from 'interface.cpp' import install_unscopeables with context %}
{% from 'interface.cpp' import install_conditionally_enabled_attributes_on_prototype with context %}
{% from 'methods.cpp' import install_conditionally_enabled_methods
        with context %}
{% if has_prepare_prototype_and_interface_object %}
void {{v8_class_or_partial}}::preparePrototypeAndInterfaceObject(v8::Local<v8::Context> context, const DOMWrapperWorld& world, v8::Local<v8::Object> prototypeObject, v8::Local<v8::Function> interfaceObject, v8::Local<v8::FunctionTemplate> interfaceTemplate)
{
#error No one is currently using a partial interface with context-dependent properties.  If you\'re planning to use it, please consult with the binding team: <blink-reviews-bindings@chromium.org>
    {{v8_class}}::preparePrototypeAndInterfaceObject(context, world, prototypeObject, interfaceObject, interfaceTemplate);
    v8::Isolate* isolate = context->GetIsolate();
{% if unscopeables %}
    {{install_unscopeables() | indent}}
{% endif %}
{% if has_conditional_attributes_on_prototype %}
    {{install_conditionally_enabled_attributes_on_prototype() | indent}}
{% endif %}
{% if methods | conditionally_exposed(is_partial) %}
    {{install_conditionally_enabled_methods() | indent}}
{% endif %}
}
{% endif %}

{% endblock %}


{##############################################################################}
{% block partial_interface %}
void {{v8_class_or_partial}}::initialize()
{
    // Should be invoked from ModulesInitializer.
    {{v8_class}}::updateWrapperTypeInfo(
        &{{v8_class_or_partial}}::install{{v8_class}}Template,
        {{prepare_prototype_and_interface_object_func}});
    {% for method in methods %}
    {% if method.overloads and method.overloads.has_partial_overloads %}
    {{v8_class}}::register{{method.name | blink_capitalize}}MethodForPartialInterface(&{{cpp_class_or_partial}}V8Internal::{{method.name}}Method);
    {% endif %}
    {% endfor %}
}

{% endblock %}
