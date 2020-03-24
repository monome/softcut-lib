switch(id) {
    {%- for op in ops %}
    case {{ op.name | cpp_enum }}:
        {%- if op.name is voice %}
        cut.voice(p->idx_0).{{op.name | cpp_voice_setter}}(
        {%- if op.format[1] == 'f' -%}
        p->value
        {%- endif -%}
        {%- if op.format[1] == 'i' -%}
        p->idx_1
        {%- endif -%}
        {%- if op.format[1] == 'i' -%}
        p->value > 0.f
        {%- endif -%}
        );
        {%- else %}
        cut.{{op.name | cpp_setter}}(/*FIXME*/);
        {%- endif %}
        break;
    {%- endfor %}
} AudioCommand;