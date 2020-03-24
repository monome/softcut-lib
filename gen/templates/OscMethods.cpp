
    {%- for op in ops %}
    addServerMethod("{{op.name | osc_path}}", "if", [](lo_arg **argv, int argc) {
        if (argc < {{ op.format| length }}) { return; }
        {% if op.is_audio_thread -%}
        Commands::softcutCommands.post({{ op.name | cpp_enum }},{{" "}}
        {%- else -%}
        softCutClient->{{op.name | cpp_setter}}(
        {%- endif -%}
        {%- for n in range(op.format | length) -%}
            argv[{{ n }}]->{{ op.format[n] }}
            {%- if n == ((op.format | length)-1) %}{%else%}, {%endif -%}
        {%- endfor -%}
        );
    });
    {% endfor %}